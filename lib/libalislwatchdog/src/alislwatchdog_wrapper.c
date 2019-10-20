/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislwatchdog_wrapper.c
 *  @brief              wrapper layer of watchdog
 *
 *  @version            1.0
 *  @date               2017/2/15 15:08:44 PM
 *  @revision           none
 */

/* System header */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* Platformlog header */
#include <alipltflog.h>
#include <alisl_types.h>

/* WATCHDOG header */
#include <alislwatchdog.h>

#include <ali_pmu_common.h>
#include <linux/version.h>
#include <linux/watchdog.h>
#include <sys/reboot.h>

#include <ali_watchdog_common.h>
/* WATCHDOG library header */
#include <alislwatchdog.h>

static int get_watchdog_fd();

static const char *dog_name = "/dev/watchdog";
static pthread_t reboot_thread;
static int reboot_thread_ret = -1;
static int g_dog_handle = -1;
static pthread_mutex_t g_dog_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_pipe[2] = {0, 0};

// reboot command work OK
// Not to reboot by stop feed watchdog
// /dev/watchdog can be open only one time, so not support multi-process
static int dog_reboot()
{
    int pmu_fd = 0;
    unsigned long wct_reboot = 0;
    //int watchdog_fd = -1;

    //int flags = WDIOS_ENABLECARD;
    //unsigned int duration_time = 1 * 1000 * 1000;
    //unsigned long ul_timeout = 0;
    int ret = 0;

    pmu_fd = open("/dev/ali_pmu", O_RDWR | O_NONBLOCK);
    if (pmu_fd < 0) {
        SL_ERR("get pmu handle failed.\n");
        //return -1;
    }

    //if ((watchdog_fd = get_watchdog_fd()) == -1) {
    //    close(pmu_fd);
    //    return -1;
    //}

    // Set the flag
    if (pmu_fd >= 0) {
        wct_reboot = ALISL_PWR_REBOOT;
        if (ioctl(pmu_fd, ALI_PMU_SAVE_WDT_REBOOT_FLAG, &wct_reboot)) {
            SL_ERR("WDT reboot flag seting error.\n");
        }
    }

// Must lock this operations to avoid feeding dog in other thread
//    alislwatchdog_set_duration_time(1 * 1000 * 1000); // us
//    alislwatchdog_start();
//    // System should have reboot, Will not come here
//    sleep(5);

    //pthread_mutex_lock(&g_dog_mutex);

//#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
//    ul_timeout = duration_time / (1000 * 1000); // us to s
//#else
//    ul_timeout = duration_time; // us
//#endif

    // WDIOC_SETTIMEOUT may not support 0 as timeout
    // So the reboot may delay 1s
    //ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &ul_timeout);
    //if (ret != 0) {
    //    SL_ERR("set dog timeout(%ld) failed!\n", ul_timeout);
    //}
    // Use watchdog to reboot to avoid that system call reboot not work
    // But watchdog reboot may not work in multi-process environment
    //ret = ioctl(watchdog_fd, WDIOC_SETOPTIONS, &flags);
    //if (ret != 0) {
    //    SL_ERR("dog start failed!\n");
    //}

    // System call reboot will finally use watchdog to reboot
    ret = reboot(RB_AUTOBOOT);
    if (ret != 0) {
        SL_ERR("system call reboot failed!\n");
    }

    sleep(5);
    // System should have reboot, Will not come here

    //pthread_mutex_unlock(&g_dog_mutex);
    if (pmu_fd >= 0) {
        wct_reboot = ALISL_PWR_WDT;
        if (ioctl(pmu_fd, ALI_PMU_SAVE_WDT_REBOOT_FLAG, &wct_reboot)) {
            SL_ERR("WDT flag seting error.\n");
        }
        close(pmu_fd);
    }
    //close(watchdog_fd);
    SL_ERR("ALISL reboot fail\n");
    return -1;
}

static void delay_reboot(void * data)
{
    SL_DBG("delay_reboot thread running\n");
    unsigned int delay = (unsigned int)data; // delay in us
    fd_set rfds;
    struct timeval tv;
    int retval = 0;

    FD_ZERO(&rfds);
    FD_SET(g_pipe[0], &rfds);

    tv.tv_sec = delay / 1000 / 1000;
    tv.tv_usec = delay % (1000 * 1000) / 1000;
    SL_DBG("Reboot delay %lds %ldus\n", tv.tv_sec, tv.tv_usec);
    while(1) {
        if ((retval = select(g_pipe[0] + 1, &rfds, NULL, NULL, &tv)) < 0) {
            SL_ERR("WDT reboot Select error.\n");
            sleep(1);
            continue;
        } else if (!retval) {
            SL_DBG("No data, Delay time is out, ALISL reboot now\n");
            //sleep(1); // debug
            dog_reboot(); // Delay time is out, reboot now
            sleep(1);
            continue;
        }

        // APP call reboot API again
        SL_DBG("Data is available, delay_reboot was Invoked again\n");
        if (FD_ISSET(g_pipe[0], &rfds)) {
            if (read(g_pipe[0], (void *)&delay, sizeof(delay)) > 0) {
               tv.tv_sec = delay / 1000 / 1000;
               tv.tv_usec = delay % (1000 * 1000) / 1000;
               SL_DBG("Reset delay %lds %ldus\n", tv.tv_sec, tv.tv_usec);
            } else {
                SL_ERR("WDT reboot read delay time error. Reboot now\n");
                dog_reboot(); // reboot now
                sleep(1);
            }
        }
    }
}

/**
 *  Function Name:      alislwatchdog_reboot
 *  @brief              watchdog reboot
 *
 *  @param              reboot delay in micro second
 *
 *  @return             alisl_retcode
 */
alisl_retcode alislwatchdog_reboot(unsigned int reboot_time)
{
    int ret = 0;

    // Use a thread to achieve delay reboot.
    // Android not support pthread_cancel, so use Pipe and Select
    // No API to cancel reboot process, so the thread will not exit.
    // Assume this API can be invoked several times.

    if (!reboot_thread_ret) {
        //pthread_cancel(reboot_thread); // Android not support pthread_cancel
        if (write(g_pipe[1], (void *)&reboot_time, sizeof(reboot_time))
            != sizeof(reboot_time)) {
            SL_ERR("Pass reboot delay time error\n");
            return -1;
        } else {
            return 0;
        }
    }

    if (pipe(g_pipe) == -1) {
        SL_ERR("create Pipe error\n");
        return -1;
    }

    // Can't use watch dog to do the delay
    // Use a thread to do the delay
    pthread_attr_t attr_reboot;
    pthread_attr_init(&attr_reboot);
    pthread_attr_setdetachstate(&attr_reboot, PTHREAD_CREATE_DETACHED);
    reboot_thread_ret = pthread_create(&reboot_thread, &attr_reboot,
        (void *)&delay_reboot, (void *)reboot_time);
    pthread_attr_destroy(&attr_reboot);
    ret = reboot_thread_ret;
    if (ret) {
       close(g_pipe[0]);
       close(g_pipe[1]);
        SL_ERR("ALISL DOG pthread_create error\n");
    }

    return ret;
}

static int get_watchdog_fd()
{
    if (g_dog_handle == -1) {
        g_dog_handle = open(dog_name, O_RDWR);
        if (g_dog_handle == -1) {
            SL_ERR("can not open dog device.\n");
            return -1;
        }
    }
    return g_dog_handle;
}

/**
 *  Function Name:      alislwatchdog_start
 *  @brief              make watchdog device to work
 *
 *  @return             alisl_retcode
 */
alisl_retcode alislwatchdog_start(void)
{
    int ret = 0;
    int watchdog_fd = -1;

    if ((watchdog_fd = get_watchdog_fd()) == -1)
        return -1;

    // dog enable
    int flags = WDIOS_ENABLECARD;

    pthread_mutex_lock(&g_dog_mutex);
    ret = ioctl(watchdog_fd, WDIOC_SETOPTIONS, &flags);
    if (ret != 0) {
        SL_ERR("dog start failed!\n");
        pthread_mutex_unlock(&g_dog_mutex);
        return -1;
    }
    pthread_mutex_unlock(&g_dog_mutex);

    return ret;
}

/**
 *  Function Name:      alislwatchdog_stop
 *  @brief              stop watchdog device
 *
 *  @return             alisl_retcode
 */
alisl_retcode alislwatchdog_stop(void)
{
    int ret = 0;
    int watchdog_fd = -1;

    if ((watchdog_fd = get_watchdog_fd()) == -1)
        return -1;

    // dog disable
    int flags = WDIOS_DISABLECARD;

    pthread_mutex_lock(&g_dog_mutex);
    ret = ioctl(watchdog_fd, WDIOC_SETOPTIONS, &flags);
    if (ret != 0) {
        SL_ERR("dog stop failed!\n");
        pthread_mutex_unlock(&g_dog_mutex);
        return -1;
    }
    pthread_mutex_unlock(&g_dog_mutex);

    return ret;
}

/**
 *  Function Name:      alislwatchdog_feed_dog
 *  @brief              feed dog to make system not to reboot
 *
 *  @param              time_us time to feed dog
 *
 *  @return             alisl_retcode
 */
alisl_retcode alislwatchdog_feed_dog(unsigned int time_us)
{
    int ret = 0;
    (void)time_us;
    int watchdog_fd = -1;

    if ((watchdog_fd = get_watchdog_fd()) == -1)
        return -1;

    // test dog keepalive
    ret = ioctl(watchdog_fd, WDIOC_KEEPALIVE, 0);

    pthread_mutex_lock(&g_dog_mutex);
    if (ret != 0) {
        SL_ERR("dog ping failed!\n");
        pthread_mutex_unlock(&g_dog_mutex);
        return -1;
    }
    pthread_mutex_unlock(&g_dog_mutex);

    return ret;
}

/**
 *  Function Name:      alislwatchdog_get_time_left
 *  @brief              get left time
 *
 *  @return             time_left left time
 */
int alislwatchdog_get_time_left(void)
{
    int ret = 0;
    unsigned long ul_timeout = 0;
    int watchdog_fd = -1;

    if ((watchdog_fd = get_watchdog_fd()) == -1)
        return -1;

    // test get dog timeout
    ret = ioctl(watchdog_fd, WDIOC_GETTIMELEFT, &ul_timeout);
    if (ret != 0) {
        SL_ERR("get dog timeout error<%ld, %ld>.\n",ul_timeout, ul_timeout);
        return -1;
    }

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
    ul_timeout = ul_timeout * (1000 * 1000); // s to us
#else
    // us
#endif

    return ul_timeout;
}

/**
 *  Function Name:      alislwatchdog_set_duration_time
 *  @brief              set timeout
 *  @param              duration_time
 *
 *  @return             NULL
 */
void alislwatchdog_set_duration_time(unsigned int duration_time)
{
    int ret = 0;
    unsigned long ul_timeout = 0;
    int watchdog_fd = -1;

    if ((watchdog_fd = get_watchdog_fd()) == -1)
        return;

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
    ul_timeout = duration_time / (1000 * 1000); // us to s
#else
    ul_timeout = duration_time; // us
#endif

    pthread_mutex_lock(&g_dog_mutex);
    ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &ul_timeout);
    if (ret != 0) {
        SL_ERR("set dog timeout(%ld) failed!\n", ul_timeout);
        pthread_mutex_unlock(&g_dog_mutex);
        return;
    }
    pthread_mutex_unlock(&g_dog_mutex);

    return;
}
