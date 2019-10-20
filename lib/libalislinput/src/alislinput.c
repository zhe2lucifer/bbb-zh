/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.c
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

//#include <types.h>
//#include <retcode.h>
//#include <osal/osal.h>
//#include <hld/hld_dev.h>
//#include <hld/pan/pan.h>
//#include <hld/pan/pan_dev.h>
//#include <hld_cfg.h>

//#include <alislinput_rfk.h>
#include "alislinput_dbg.h"

#include <alislinput.h>
#include <alipltfretcode.h>
#include <alipltflog.h>
#include <alislevent.h>
#include <sys/epoll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <linux/input.h>
#include <ali_front_panel_common.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#define PAN_DBG_LEVEL_HLD 0

#ifndef HLD_MAX_NAME_SIZE
#define HLD_MAX_NAME_SIZE 16
#endif

#ifndef HLD_DEV_TYPE_PAN
#define HLD_DEV_TYPE_PAN		0x01070000	/* Front panel device */
#endif

#ifndef OSAL_WAIT_FOREVER_TIME
#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF
#endif

#define MAX_KUMSG_SIZE 1024

struct alislinput_device *g_dev;

static unsigned char g_alislinput_repeat_enable = 1;
static unsigned long g_alislinput_ir_format = 0xff;

static char g_alislinput_front_panel_name[HLD_MAX_NAME_SIZE] = CH455_DEV_NAME;
static char g_alislinput_display_backup[256] = {0};

static long g_alislinput_panel_handle = -1;
static long g_alislinput_ir_handle = -1; // file handle of /dev/ali_ir
static long g_alislinput_input_handle_ir = -1; //ir, the file handle of /dev/input/event0
static long g_alislinput_input_handle_panel = -1; //panel
static alislinput_device_status_e g_alislinput_device_status = PAN_DEV_DETACH;

static unsigned long g_alislinput_rfk_port_ir = 0;
static unsigned long g_alislinput_rfk_port_pan = 0;

static struct alislinput_key	g_alislinput_key = {0, 0, 0, 0};				/* Current input key */
static struct alislinput_key_index g_alislinput_key_index = {{0, 0, 0, 0}, 0}; 		/* Current input key and key value index*/
static struct alislinput_key_info	g_alislinput_key_info = {0, 0, 0, 0, 0};		/* Current input key */
static struct ali_fp_key_map_cfg g_alislinput_ir_key_map = {NULL, 0, 0, 0, 0, 0};		/* ir key map */
static struct ali_fp_key_map_cfg g_alislinput_panel_key_map = {NULL, 0, 0, 0, 0, 0};		/* panel key map */

static struct alislevent g_ir_slev;
static struct alislevent g_panel_slev;
static struct alislevent g_ir_slev_k;
static struct alislevent g_panel_slev_k;
static long g_ir_socket_id; //kumsgfd for ir
static long g_panel_socket_id; //kumsgfd for panel
static void *g_ev_handle;
static int g_phy_code;

/* dbg info */
static struct alislinput_dbg g_alislinput_dbg;

#define PAN_MAX_PATH_SIZE		32
#if 0
#define IR_PRINTF(fmt, args...)				\
{										\
	if (0 !=  (g_alislinput_dbg.ir_level & PAN_DBG_LEVEL_HLD))				\
	{									\
		AUI_DBG(fmt, ##args);					\
	}									\
}

#define PAN_PRINTF(fmt, args...)				\
{										\
	if ((0 !=  (g_alislinput_dbg.ir_level & PAN_DBG_LEVEL_HLD)) || \
		(0 !=  (g_alislinput_dbg.panel_level &PAN_DBG_LEVEL_HLD)))	\
	{									\
		AUI_DBG(fmt, ##args);					\
	}									\
}


#define PAN_KEY_PRINTF(type, fmt, args...)				\
{										\
	if (((0 !=  (g_alislinput_dbg.ir_level & PAN_DBG_LEVEL_HLD)) && (type == PAN_KEY_TYPE_REMOTE)) || \
		((0 !=  (g_alislinput_dbg.panel_level &PAN_DBG_LEVEL_HLD)) && (type == PAN_KEY_TYPE_PANEL)))	\
	{									\
		AUI_DBG(fmt, ##args);					\
	}									\
}
#endif
long alislinput_get_panel_name(char *dev_name)
{
	long fd = -1;
	long ret = 0;
	long read_size = 0;
	char buffer[HLD_MAX_NAME_SIZE];


	if (NULL == dev_name)
	{
		SL_ERR("Invalid parameters!\n");
		return -1;
	}

	memset(buffer, 0x00, sizeof(buffer));
	fd = open("/proc/panel", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		SL_ERR(" open /proc/panel Fail!\n");
		return -1;
	}

	read_size = read(fd, buffer, sizeof(buffer));
	if (-1 == read_size)
	{
		SL_ERR("read Fail!\n");
		ret = -1;
	}
	else
	{
		SL_DBG("dev name = %s\n", buffer);
		if (read_size > (long)sizeof(buffer))
		{
			read_size =  (long)sizeof(buffer);
		}
		memcpy(dev_name, buffer, read_size);
	}

	close(fd);

	return ret;
}

void *alislinput_dev_alloc(char *name, unsigned long type, unsigned long size)
{
	struct alislinput_device *dev = (struct alislinput_device *)malloc(size);
	//struct alislinput_device *dp;
	unsigned short id = 0;

	if (dev == NULL)
	{
		SL_ERR("hld_dev_alloc: error - device %s not enough memory: %08x!\n",
			  name, (unsigned int)size);
		return NULL;
	}

//	/* Name same with some one in HLD device list, error */
//	for (id = 0, dp = hld_device_base; dp != NULL; dp = dp->next)
//	{
//		/* Find the device */
//		if (strcmp(dp->name, name) == 0)
//		{
//			PRINTF("hld_dev_alloc: error - device %s same name!\n", name);
//			free(dev);
//			return NULL;
//		}
//		/* Check ID */
//		if ((dp->type & HLD_DEV_TYPE_MASK) == type)
//		{
//			id++;
//		}
//	}

	/* Clear device structure */
	memset((unsigned char *)dev, 0, size);

	dev->type = (type | id);
	strcpy(dev->name, name);

	return dev;
}

long alislinput_dev_register(void *dev)
{
//	return hld_dev_add((struct hld_device *)dev);
	if (dev == NULL)
		return -1;
	return 0;
}

void alislinput_dev_free(void *dev)
{
	if (dev != NULL)
	{
//		hld_dev_remove((struct hld_device *)dev);
		free(dev);
	}

	return;
}

long alislinput_attach(void)
{
	char dev_name[HLD_MAX_NAME_SIZE];
	struct alislinput_device *dev = NULL;
	long ret = 0;


	if (PAN_DEV_DETACH != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d(want %d)\n",
			g_alislinput_device_status, PAN_DEV_ATTACH);

		return (PAN_DEV_ATTACH == g_alislinput_device_status) ? 0 : -1;
	}


	memset(dev_name, 0x00, sizeof(dev_name));
	if (0 != alislinput_get_panel_name(dev_name))
	{
		SL_ERR("No Panel!\n");
		strcpy(g_alislinput_front_panel_name, "No Panel");
		//return -1;
	}
	else
	{
		strcpy(g_alislinput_front_panel_name, dev_name);
		SL_DBG("panel = %s\n", g_alislinput_front_panel_name);
	}


	(dev) = alislinput_dev_alloc(g_alislinput_front_panel_name, HLD_DEV_TYPE_PAN, sizeof(struct alislinput_device));
//	if ((dev) == NULL){
//		SL_ERR("%s error: Alloc front panel device error!\n", __FUNCTION__);
//		return -1;
//	}

	/* Add this device to queue */
	if ((ret = alislinput_dev_register((dev))) != 0){
		SL_ERR("error %d: Register front panel device error!\n", ret);
		alislinput_dev_free((dev));
		return -1;
	}

	memset(&g_alislinput_ir_key_map, 0, sizeof(g_alislinput_ir_key_map));
	memset(&g_alislinput_panel_key_map, 0, sizeof(g_alislinput_panel_key_map));

	//snprintf((dev)->name, sizeof((dev)->name), "/dev/%s", dev_name);
	SL_DBG("panel path = %s\n", (dev)->name);

//	alislinput_module_register();

	g_alislinput_device_status = PAN_DEV_ATTACH;

	/* add for test */
	g_alislinput_dbg.ir_level = PAN_DBG_LEVEL_HLD;
	g_alislinput_dbg.panel_level = PAN_DBG_LEVEL_HLD;

	g_dev = dev;
	return 0;
}


void alislinput_detach(struct alislinput_device *dev)
{
	if ((PAN_DEV_CLOSE != g_alislinput_device_status) && (PAN_DEV_ATTACH != g_alislinput_device_status))
	{
		SL_WARN("status error, now = %d(want %d)\n",
			g_alislinput_device_status, PAN_DEV_DETACH);

		return;
	}


	if(g_alislinput_ir_key_map.map_entry){
		free(g_alislinput_ir_key_map.map_entry);
		g_alislinput_ir_key_map.map_entry = NULL;
	}

	if(g_alislinput_panel_key_map.map_entry){
		free(g_alislinput_panel_key_map.map_entry);
		g_alislinput_panel_key_map.map_entry = NULL;
	}
	if (dev)
	{
		alislinput_dev_free(dev);
	}

	g_alislinput_device_status = PAN_DEV_DETACH;
}


long alisl_input_alislinput_get_input_name(char *dev, char *input_name)
{
	char *position = NULL;
	char *end = NULL;
	char *pEnd = NULL;
	char buffer[1024];
	long fd = -1;
	long ret = -1;
	long read_size = 0;


	if (NULL == dev)
	{
		SL_ERR("device_name is NULL\n");
		return -1;
	}

	SL_DBG("dev = %s\n", dev);

	memset(buffer, 0x00, sizeof(buffer));
	fd = open("/proc/bus/input/devices", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		SL_ERR("open /proc/bus/input/devices Fail!\n");
		return -1;
	}

	read_size = read(fd, buffer, sizeof(buffer));
	if (-1 == read_size)
	{
		SL_ERR("read Fail!\n");
		ret = -1;
		goto END;
	}

	position = strstr(buffer, dev);
	if (NULL == position)
	{
		SL_ERR("Fail!, dev = %s\n", dev);
		ret = -1;
		goto END;
	}

	position = strstr(position, "Handlers");
	if (NULL == position)
	{
		SL_ERR("Fail to find Handlers.\n");
		ret = -1;
		goto END;
	}

	position = strstr(position, "event");
	if (NULL == position)
	{
		SL_ERR("Fail to find event!\n");
		ret = -1;
		goto END;
	}

	if (strstr(position, " "))
	{
		end = strstr(position, " ");
	}
	else
	{
		end = strstr(position, "\n");
	}
	if (NULL != end)
	{
		if ((end - position) <= 16)
		{
			memcpy(input_name, position, end - position);
		}
		else
		{
			memcpy(input_name, position, 16);
		}
		// return the event id
		ret = strtol(input_name+5, &pEnd, 10); //the input_name = "eventN"
		if (ret == 0 && pEnd == input_name)
			ret = -1;
		SL_DBG("input_name = %s, ret: %d\n", input_name, (int)ret);
	}
	else
	{
		SL_ERR("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto END;
	}

//	ret = 0;


	END:
	{
		close(fd);
		return ret;
	}
}


/*
 * 	Name		:   alislinput_open()
 *	Description	:   Open a pan device
 *	Parameter	:	struct alislinput_device *dev		: Device to be openned
 *	Return		:	long 						: Return value
 *
 */
long alislinput_open(struct alislinput_device **dev)
{
	long ret_ir = 0;
	long ret_panel = 0;
//	long ret_pmu_key = 0;
	char input_name[HLD_MAX_NAME_SIZE];
	char input_path[PAN_MAX_PATH_SIZE];
	char panel_path[PAN_MAX_PATH_SIZE];
	//long flags = 0;
	long event_id = 0;


	if ((PAN_DEV_ATTACH != g_alislinput_device_status) && (PAN_DEV_CLOSE != g_alislinput_device_status))
	{
		SL_WARN("status error, now = %d(want %d)\n", 
			g_alislinput_device_status, PAN_DEV_OPEN);

		return (PAN_DEV_OPEN == g_alislinput_device_status) ? 0 : -1;
	}

	if(NULL==dev){
		SL_ERR("error:  NULL device node!\n");
		return -1;
	}

	SL_DBG("dev->name %s\n", g_dev->name);

	g_alislinput_ir_handle = open("/dev/ali_ir",O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if(g_alislinput_ir_handle<0){
		SL_ERR("open %s failed.\n",  "/dev/ali_ir_g2");
		ret_ir |= -1;
	}

	snprintf(panel_path, sizeof(panel_path), "/dev/%s", g_dev->name);
	g_alislinput_panel_handle = open(panel_path,O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if(g_alislinput_panel_handle<0){
		SL_ERR("open %s failed.\n",  panel_path);
		ret_panel |= -1;
	}

	memset(input_path, 0x00, sizeof(input_path));
	memset(input_name, 0x00, sizeof(input_name));

	if (((event_id = alisl_input_alislinput_get_input_name("ali_ir", input_name)) >= 0)
		|| (event_id = (alisl_input_alislinput_get_input_name("lircd", input_name)) >= 0))
	{

		//snprintf(input_path, sizeof(input_path), "/dev/input/%s", input_name);
		snprintf(input_path, sizeof(input_path), "/dev/input/event%ld", event_id);
		SL_DBG("input_path = %s\n", input_path);
		g_alislinput_input_handle_ir = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (g_alislinput_input_handle_ir < 0)
		{
			//snprintf(input_path, sizeof(input_path), "/dev/%s", input_name);
			snprintf(input_path, sizeof(input_path), "/dev/event%ld", event_id);
			SL_INFO("input_path = %s\n", input_path);
			g_alislinput_input_handle_ir = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
			if (g_alislinput_input_handle_ir < 0)
			{
				SL_ERR("failed: g_alislinput_input_handle_ir %s\n", strerror(errno));
				ret_ir |= -1;
			}
		}
	}


    //SL_DBG("\n to get ir kumsgfd with: %d\n", g_alislinput_ir_handle);
    int flags = O_CLOEXEC;
    g_alislinput_rfk_port_ir = ioctl(g_alislinput_ir_handle, ALI_FP_GET_KUMSGQ, &flags);      
    if(g_alislinput_rfk_port_ir == 0)
    {
        SL_ERR("rfk get port fail\n");
        ret_ir |= -1;
    }
    else
    {
        //SL_DBG("get kumsgfd for ir: %d\n", g_alislinput_rfk_port_ir);
        g_ir_socket_id = g_alislinput_rfk_port_ir;
    }


	memset(input_path, 0x00, sizeof(input_path));
	memset(input_name, 0x00, sizeof(input_name));
	if ((event_id = alisl_input_alislinput_get_input_name(g_alislinput_front_panel_name, input_name)) >= 0)
	{

		//snprintf(input_path, sizeof(input_path), "/dev/input/%s", input_name);
		snprintf(input_path, sizeof(input_path), "/dev/input/event%ld", event_id);
		SL_DBG("input_path = %s\n", input_path);
		g_alislinput_input_handle_panel = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (g_alislinput_input_handle_panel < 0)
		{
			//snprintf(input_path, sizeof(input_path), "/dev/%s", input_name);
			snprintf(input_path, sizeof(input_path), "/dev/event%ld", event_id);
			SL_DBG("input_path = %s\n", input_path);

			g_alislinput_input_handle_panel = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
			if (g_alislinput_input_handle_panel < 0)
			{
				SL_ERR("failed: g_alislinput_input_handle_panel %s\n", strerror(errno));
				ret_panel |= -1;
			}
		}
	}

    //SL_DBG("\n to get panel kumsgfd with: %d\n", g_alislinput_panel_handle);
    flags = O_CLOEXEC;
    g_alislinput_rfk_port_pan = ioctl(g_alislinput_panel_handle, ALI_FP_GET_KUMSGQ, &flags);      
    if(g_alislinput_rfk_port_pan == 0)
    {
        SL_ERR("rfk get port fail\n");
        ret_panel |= -1;
    }
    else
    {
        //SL_DBG("get kumsgfd for panel: %d\n", g_alislinput_rfk_port_pan);
        g_panel_socket_id = g_alislinput_rfk_port_pan;
    }

	g_alislinput_display_backup[0] = 0;
	memset(&g_alislinput_key, 0x00, sizeof(g_alislinput_key));
	memset(&g_alislinput_key_info, 0x00, sizeof(g_alislinput_key_info));
	memset(&g_alislinput_dbg, 0x00, sizeof(g_alislinput_dbg));
	g_alislinput_dbg.interval = 5000;

	/* add for test */
	g_alislinput_dbg.ir_level = PAN_DBG_LEVEL_HLD;
	g_alislinput_dbg.panel_level = PAN_DBG_LEVEL_HLD;

	/* Setup init work mode */
	if ((0 == ret_ir) || (0 == ret_panel))
	{
		g_alislinput_device_status = PAN_DEV_OPEN;
		g_ir_slev.fd = g_alislinput_input_handle_ir;
        /* 
            for AUI_Support_Window #54973, remove the "EPOLLET", 
            to avoid key callback delay.
        */
		g_ir_slev.events = EPOLLIN;// | EPOLLET; 
		g_ir_slev.data = (void *)0;
		g_panel_slev.fd = g_alislinput_input_handle_panel;
		g_panel_slev.events = EPOLLIN ;//| EPOLLET;
		g_panel_slev.data = (void *)1;
		g_ir_slev_k.fd = g_ir_socket_id;
		g_ir_slev_k.events = EPOLLIN;// | EPOLLET;
		g_ir_slev_k.data = (void *)2;
		g_panel_slev_k.fd = g_panel_socket_id;
		g_panel_slev_k.events = EPOLLIN;// | EPOLLET;
		g_panel_slev_k.data = (void *)3;
	//	SL_DBG("alislinput_open ir %d, panel: %d\n", g_ir_slev.fd, g_panel_slev.fd);
	//	SL_DBG("alislinput_open ir_k %d, panel_k: %d\n", g_ir_slev_k.fd, g_panel_slev_k.fd);
	}
	else
	{
	    if (g_ir_socket_id) {
            close(g_ir_socket_id);
            g_ir_socket_id = 0;
        }
        if (g_panel_socket_id) {
            close(g_panel_socket_id);
            g_panel_socket_id = 0;
        }
		if(g_alislinput_input_handle_ir >= 0)
		{
			close(g_alislinput_input_handle_ir);
			g_alislinput_input_handle_ir = -1;
		}
		if(g_alislinput_input_handle_panel >= 0)
		{
			close(g_alislinput_input_handle_panel);
			g_alislinput_input_handle_panel = -1;
		}
		if(g_alislinput_ir_handle >= 0)
		{
			close(g_alislinput_ir_handle);
			g_alislinput_ir_handle = -1;
		}
		if(g_alislinput_panel_handle >= 0)
		{
			close(g_alislinput_panel_handle);
			g_alislinput_panel_handle = -1;
		}
	}

	*dev = g_dev;
	return (ret_ir || ret_panel);
}


/*
 * 	Name		:   alislinput_close()
 *	Description	:   Close a pan device
 *	Parameter	:	struct alislinput_device *dev		: Device to be closed
 *	Return		:	long 						: Return value
 *
 */
long alislinput_close(struct alislinput_device *dev)
{
	long result = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_WARN("status error, now = %d(want %d)\n", 
			g_alislinput_device_status, PAN_DEV_CLOSE);

		return (PAN_DEV_CLOSE == g_alislinput_device_status) ? 0 : -1;
	}

//	PAN_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);

	if(NULL==dev){
		SL_ERR("error:  NULL device node!\n");
		return -1;
	}

	/* Stop device */
    if (g_ir_socket_id) {
        close(g_ir_socket_id);
        g_ir_socket_id = 0;
    }
    if (g_panel_socket_id) {
        close(g_panel_socket_id);
        g_panel_socket_id = 0;
    }
	if(g_alislinput_input_handle_ir >= 0)
	{
		close(g_alislinput_input_handle_ir);
		g_alislinput_input_handle_ir = -1;
	//	SL_DBG("close g_alislinput_input_handle_ir\n");
	}
	if(g_alislinput_input_handle_panel >= 0)
	{
		close(g_alislinput_input_handle_panel);
		g_alislinput_input_handle_panel = -1;
//		SL_DBG("close g_alislinput_input_handle_panel\n");
	}
	if(g_alislinput_ir_handle >= 0)
	{
		close(g_alislinput_ir_handle);
		g_alislinput_ir_handle = -1;
//		SL_DBG("close g_alislinput_ir_handle\n");
	}
	if(g_alislinput_panel_handle >= 0)
	{
		close(g_alislinput_panel_handle);
		g_alislinput_panel_handle = -1;
//		SL_DBG("close g_alislinput_panel_handle\n");
	}
#if 0
	if (g_alislinput_rfk_port_ir > 0)
	{
		alislinput_rfk_free_port(g_alislinput_rfk_port_ir);
	}

	if (g_alislinput_rfk_port_pan > 0)
	{
		alislinput_rfk_free_port(g_alislinput_rfk_port_pan);
	}
#endif

	/* Update flags */
	g_alislinput_device_status = PAN_DEV_CLOSE;


	return result;
}


/*
phy_code :
		0, logic;
		1, index.
*/
long alislinput_config_key_map(struct alislinput_device *dev, unsigned char phy_code, unsigned char *map, unsigned long map_len, unsigned long unit_len)
{
	long ir_rlt = 0;
	unsigned char * map_entry;

	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	if ((NULL== map) && (2 != phy_code))
	{
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	SL_DBG("phy_code = %d, map_len = 0x%x, unit_len = 0x%x\n", 	phy_code,(unsigned int) map_len, (unsigned int)unit_len);

	if (g_phy_code != phy_code) {
		// Remove the previous epoll fds
		if (g_ir_slev.cb != NULL && g_phy_code == 2) {
			alislevent_del(g_ev_handle, &g_ir_slev_k);
			alislevent_del(g_ev_handle, &g_panel_slev_k);
			g_ir_slev_k.cb = NULL;
			g_panel_slev_k.cb = NULL;
		}
		if (g_ir_slev_k.cb != NULL && g_phy_code != 2) {
			alislevent_del(g_ev_handle, &g_ir_slev);
			alislevent_del(g_ev_handle, &g_panel_slev);
			g_ir_slev.cb = NULL;
			g_panel_slev.cb = NULL;
		}
	}
	g_phy_code = phy_code;
	g_alislinput_ir_key_map.phy_code = phy_code;
	if (2 != phy_code)
	{
		SL_DBG("alislinput_config_key_map 2 != phy_code\n");
		map_entry = malloc(map_len+g_alislinput_ir_key_map.map_len);
		if(NULL==map_entry){
			SL_ERR("error: no memory!\n");
			return -1;
		}
		/* backup key map */
		if (g_alislinput_ir_key_map.map_entry)
		{
			memcpy(map_entry, g_alislinput_ir_key_map.map_entry, g_alislinput_ir_key_map.map_len);
			memcpy(map_entry+g_alislinput_ir_key_map.map_len, map, map_len);
		}
        else
        {
			memcpy(map_entry, map, map_len);
        }

		free(g_alislinput_ir_key_map.map_entry);
		g_alislinput_ir_key_map.map_entry = NULL;

		g_alislinput_ir_key_map.map_len += map_len;
		g_alislinput_ir_key_map.unit_len = unit_len;
		g_alislinput_ir_key_map.unit_num += (map_len/unit_len);
		g_alislinput_ir_key_map.map_entry = map_entry;

		if (0 !=  g_alislinput_dbg.ir_level)
		{
			long i ;

			for(i=0; i<(long)g_alislinput_ir_key_map.unit_num; i++)
			{
				unsigned char * buf = &map_entry[i*g_alislinput_ir_key_map.unit_len];
				unsigned long phy ;
				unsigned short log;
				phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
				log = buf[4]|(buf[5]<<8);
				SL_DBG("%08x	%04x\n",(unsigned int) phy, log);
			}
			SL_DBG("\n");
		}
	}

	ir_rlt = ioctl(g_alislinput_ir_handle, ALI_FP_CONFIG_KEY_MAP, (unsigned long)(&g_alislinput_ir_key_map));
	if (0 != ir_rlt)
	{
		SL_ERR("ALI_FP_CONFIG_KEY_MAP fail, return: %d\n", ir_rlt);
		return -1;
	}
	return 0;
}


long alislinput_del_key_map(struct alislinput_device *dev)
{
	long ir_rlt = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	if(g_alislinput_ir_key_map.map_entry)
	{
		free(g_alislinput_ir_key_map.map_entry);
		g_alislinput_ir_key_map.map_entry = NULL;
		memset(&g_alislinput_ir_key_map, 0, sizeof(g_alislinput_ir_key_map));
	}

	return ir_rlt;
}


/*
phy_code : 0, logic; 1, index.
*/
long alislinput_config_panel_map(struct alislinput_device *dev, unsigned char phy_code, unsigned char *map, unsigned long map_len, unsigned long unit_len)
{
	long alislinput_rlt = 0;
	unsigned char * map_entry;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	if ((NULL== map) && (2 != phy_code))
	{
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	SL_DBG("phy_code = %s, map_len = 0x%x, unit_len = 0x%x\n",
		(0 == phy_code) ? "logic" : "index", map_len, unit_len);

	if (g_phy_code != phy_code) {
		// Remove the previous epoll fds
		if (g_ir_slev.cb != NULL && g_phy_code == 2) {
			alislevent_del(g_ev_handle, &g_ir_slev_k);
			alislevent_del(g_ev_handle, &g_panel_slev_k);
			g_ir_slev_k.cb = NULL;
			g_panel_slev_k.cb = NULL;
		}
		if (g_ir_slev_k.cb != NULL && g_phy_code != 2) {
			alislevent_del(g_ev_handle, &g_ir_slev);
			alislevent_del(g_ev_handle, &g_panel_slev);
			g_ir_slev.cb = NULL;
			g_panel_slev.cb = NULL;
		}
	}

	g_phy_code = phy_code;
	g_alislinput_panel_key_map.phy_code = phy_code;
	if (2 != phy_code)
	{
		map_entry = malloc(map_len);
		if(NULL==map_entry){
			SL_ERR("error: no memory!\n");
			return -1;
		}
		memcpy(map_entry, map, map_len);
		if(g_alislinput_panel_key_map.map_entry)
		{
			free(g_alislinput_panel_key_map.map_entry);
		}
		g_alislinput_panel_key_map.map_entry = NULL;
		g_alislinput_panel_key_map.map_len = map_len;
		g_alislinput_panel_key_map.unit_len = unit_len;
		g_alislinput_panel_key_map.unit_num = (map_len/unit_len);
		g_alislinput_panel_key_map.map_entry = map_entry;
		if (0 !=  g_alislinput_dbg.panel_level)
		{
			long i ;

			for(i=0; i<(long)g_alislinput_panel_key_map.unit_num; i++)
			{
				unsigned char * buf = &map_entry[i*g_alislinput_panel_key_map.unit_len];
				unsigned long phy ;
				unsigned short log;
				phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
				log = buf[4]|(buf[5]<<8);
				SL_DBG("%08x	%04x\n", buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24), buf[4]|(buf[5]<<8));
                                (void)phy;
                                (void)log;
			}
			SL_DBG("\n");
		}
	}

	alislinput_rlt = ioctl(g_alislinput_panel_handle, ALI_FP_CONFIG_KEY_MAP, (unsigned long)(&g_alislinput_panel_key_map));
	if(0!=alislinput_rlt)
	{
		SL_ERR("fail, pan %d\n", alislinput_rlt);
		return -1;
	}

	return 0;
}


struct alislinput_key_info *  alislinput_get_key_info(struct alislinput_device *dev, unsigned long timeout)
{
	unsigned char msg_buf[MAX_KUMSG_SIZE] = {0};
	int ret = -1;
//	long size = sizeof(g_alislinput_key_info);


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return NULL;
	}

	if (NULL== dev)
	{
		SL_ERR("error:  Invalid parameters!\n");
		return NULL;
	}


	while(1)
	{
		//msg_buf = (unsigned char *)alislinput_rfk_receive_msg(g_alislinput_rfk_port_ir);
        ret = read(g_ir_socket_id, msg_buf, MAX_KUMSG_SIZE);
		if (ret > 0) //if(NULL != msg_buf)
		{
			g_alislinput_key_info.type = PAN_KEY_TYPE_REMOTE;
			goto got_key_info;
		}

		//msg_buf = (unsigned char *)alislinput_rfk_receive_msg(g_alislinput_rfk_port_pan);
		ret = read(g_panel_socket_id, msg_buf, MAX_KUMSG_SIZE);
		if (ret > 0) //if(NULL != msg_buf)
		{
			g_alislinput_key_info.type = PAN_KEY_TYPE_PANEL;
			goto got_key_info;
		}

		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}
		usleep(1 * 1000);
	}

	return NULL;


	got_key_info:
	{
		g_alislinput_key_info.code_high = (msg_buf[0] << 24) |(msg_buf[1] << 16) | (msg_buf[2] << 8) | (msg_buf[3]);
		g_alislinput_key_info.code_low =  (msg_buf[4] << 24) |(msg_buf[5] << 16) | (msg_buf[6] << 8) | (msg_buf[7]);
		g_alislinput_key_info.protocol = (alislinput_ir_protocol_e)((msg_buf[8] << 24) |(msg_buf[9] << 16)
								| (msg_buf[10] << 8) | (msg_buf[11]));
		g_alislinput_key_info.state = (alislinput_key_press_e)((msg_buf[12] << 24) |(msg_buf[13] << 16)
								| (msg_buf[14] << 8) | (msg_buf[15]));

		SL_DBG("code: 0x%08x 0x%08x, protocol: %d, state: %d, type: %s\n\n", 
					(unsigned int)g_alislinput_key_info.code_high,
					(unsigned int) g_alislinput_key_info.code_low, 
					g_alislinput_key_info.protocol,
					g_alislinput_key_info.state, 
					(PAN_KEY_TYPE_PANEL== g_alislinput_key_info.type) ? "Panel" : "IR");


		return &g_alislinput_key_info;
	}
}

struct alislinput_key * alislinput_get_key(struct alislinput_device *dev, unsigned long timeout)
{
	struct input_event key_event;
	long read_len = sizeof(struct input_event);



	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return NULL;
	}

	if(NULL==dev)
	{
		SL_ERR("error:  Invalid parameters!\n");
		return NULL;
	}

    /* User set panel key map by Panel module, so the g_alislinput_panel_key_map may be NULL.  */
	if ((NULL == g_alislinput_ir_key_map.map_entry)/* && (NULL == g_alislinput_panel_key_map.map_entry)*/)
	{
		SL_ERR("pan device not config key mapping yet!\n");
		return NULL;
	}

	while(1)
	{
		memset(&key_event, 0x00, sizeof(struct input_event));

        /* User set panel key map by Panel module, if user set ir key map first and then set panel kep map,
         * the g_alislinput_panel_key_map would be NULL.  
         * So we only check if the driver report the panel key.   
        */
		if (/*(NULL != g_alislinput_panel_key_map.map_entry) && */(read_len == read(g_alislinput_input_handle_panel, &key_event, read_len))
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{
			SL_DBG("Panel, type: %d, code: %d, value: %d, panel.phy_code: %d\n",
				   key_event.type, key_event.code, key_event.value,(unsigned int)g_alislinput_panel_key_map.phy_code);

			g_alislinput_key.type = PAN_KEY_TYPE_PANEL;
			goto got_key;
		}

		if ((NULL != g_alislinput_ir_key_map.map_entry) && (read_len == read(g_alislinput_input_handle_ir, &key_event, read_len))
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{
			SL_DBG("IR, type: %d, code: %d, value: %d, ir.phy_code: %d\n",
				   key_event.type, key_event.code, key_event.value,(unsigned int)g_alislinput_ir_key_map.phy_code);

			g_alislinput_key.type = PAN_KEY_TYPE_REMOTE;
			goto got_key;
		}


		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}

		usleep(1 * 1000);
	}

	//return auto_change_channel_check();
	return NULL;


got_key:
	if((1 == g_alislinput_ir_key_map.phy_code) && (PAN_KEY_TYPE_REMOTE == g_alislinput_key.type))
	{
		/* ir index, phy key */
		if (g_alislinput_ir_key_map.map_entry)
		{
			unsigned char buf[4];
			memcpy(buf, &g_alislinput_ir_key_map.map_entry[key_event.code*g_alislinput_ir_key_map.unit_len],
				sizeof(buf));
			g_alislinput_key.code = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
		}
	}
	else if((1 == g_alislinput_panel_key_map.phy_code) && (PAN_KEY_TYPE_PANEL== g_alislinput_key.type))
	{
		/* panel index, phy key */
		if (g_alislinput_panel_key_map.map_entry)
		{
			unsigned char buf[4];
			memcpy(buf, &g_alislinput_panel_key_map.map_entry[key_event.code*g_alislinput_panel_key_map.unit_len],
				sizeof(buf));
			g_alislinput_key.code = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
			g_alislinput_key.code |= 0xFFFF0000;
		}
	}
	else if((0 == g_alislinput_ir_key_map.phy_code) || (0 == g_alislinput_panel_key_map.phy_code))
	{
		/* logic key */
		g_alislinput_key.code = key_event.code;
	}
	else
	{
		SL_ERR("device config key mapping error!\n");
	}


	if(0==key_event.value)
	{
		g_alislinput_key.state = PAN_KEY_RELEASE;
		g_alislinput_key.count = 0;
	}
	else if (1==key_event.value)
	{
		g_alislinput_key.state = PAN_KEY_PRESSED;
		g_alislinput_key.count ++;
	}
	else
	{
		/* enable repeat key? */
		if (1 == g_alislinput_repeat_enable)
		{
			g_alislinput_key.state = PAN_KEY_PRESSED;
			g_alislinput_key.count ++;
		}
		else
		{
			return NULL;
		}
	}

	SL_DBG(" %s, type: %d, code: 0x%08x, state: %s\n", 
				(PAN_KEY_TYPE_PANEL== g_alislinput_key.type) ? "Panel" : "IR",
				key_event.type, (unsigned int)g_alislinput_key.code,
				(PAN_KEY_PRESSED == g_alislinput_key.state) ? "pressed" : "release");

//	auto_change_channel_check();

	return &g_alislinput_key;
}


struct alislinput_key_index * alislinput_get_key_index(struct alislinput_device *dev, unsigned long timeout)
{
	struct input_event key_event;
	unsigned char got_alislinput_key = 0;
	long read_len = sizeof(struct input_event);


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n",
			g_alislinput_device_status);

		return NULL;
	}

	if(NULL== dev)
	{
		SL_ERR("error:  Invalid parameters!\n");
		return NULL;
	}


	if(NULL==g_alislinput_ir_key_map.map_entry)
	{
		SL_ERR("error: device not config key mapping yet!\n");
		return NULL;
	}
	while(1)
	{
		memset(&key_event, 0x00, sizeof(struct input_event));
		got_alislinput_key = 0;

		if ((read_len == read(g_alislinput_input_handle_panel, &key_event, read_len))
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{
			SL_DBG("Panel, type: %d, code: %d, value: %d\n", 
				key_event.type, key_event.code, key_event.value);

			got_alislinput_key = 1;
			goto got_key;
		}

		if ((read_len == read(g_alislinput_input_handle_ir, &key_event, read_len))
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{
			SL_DBG("IR, type: %d, code: %d, value: %d\n",
				key_event.type, key_event.code, key_event.value);

			goto got_key;
		}

		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}

		usleep(1 * 1000);
	}

	return NULL;


got_key:
	if((1 == g_alislinput_ir_key_map.phy_code) || (1 == got_alislinput_key))
	{
		/* phy key */
		unsigned char buf[4];
		memcpy(buf, &g_alislinput_ir_key_map.map_entry[key_event.code*g_alislinput_ir_key_map.unit_len],
			sizeof(buf));
		g_alislinput_key_index.key_value.code= buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
		/* index */
		g_alislinput_key_index.key_index = key_event.code;
	}
	else
	{
		/* logic key */
		g_alislinput_key_index.key_value.code = key_event.code;
		g_alislinput_key_index.key_index = key_event.code;
	}

	if(0==key_event.value)
	{
		g_alislinput_key_index.key_value.state = PAN_KEY_RELEASE;
		g_alislinput_key_index.key_value.count = 0;
	}
	else if (1==key_event.value)
	{
		g_alislinput_key_index.key_value.state = PAN_KEY_PRESSED;
		g_alislinput_key_index.key_value.count ++;
	}
	else
	{
		/* enable repeat key? */
		if (1 == g_alislinput_repeat_enable)
		{
			g_alislinput_key_index.key_value.state = PAN_KEY_PRESSED;
			g_alislinput_key_index.key_value.count ++;
		}
		else
		{
			return NULL;
		}
	}

	SL_DBG("%s, type: %d, code: 0x%08x, index : 0x%04x, state: %s, count: %d\n",				
				(1 == got_alislinput_key) ? "Panel" : "IR",
				key_event.type,(unsigned int)g_alislinput_key_index.key_value.code, g_alislinput_key_index.key_index,
				(PAN_KEY_PRESSED == g_alislinput_key_index.key_value.state) ? "pressed" : "release",
				(unsigned int)g_alislinput_key_index.key_value.count);


	return &g_alislinput_key_index;
}


long alislinput_send_data(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout)
{
	struct ali_fp_data_transfer_param tx_param;
	long rlt = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev||NULL==data){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	memset(&tx_param, 0, sizeof(tx_param));
	tx_param.tx_buf = data;
	tx_param.tx_len = len;
	tx_param.tmo = timeout;
	rlt = ioctl(g_alislinput_panel_handle, ALI_FP_DATA_TRANSFER, (unsigned long)(&tx_param));
	if(0==rlt){
		while(timeout--){
			rlt = ioctl(g_alislinput_panel_handle, ALI_FP_TRANSFER_CHECK, 1);
			if(0==rlt)
				break;
			usleep(1 * 1000);
		}
	}
	return rlt;
}


long alislinput_receive_data(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout)
{
	struct ali_fp_data_transfer_param rx_param;
	long rlt = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev||NULL==data){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	memset(&rx_param, 0, sizeof(rx_param));
	rx_param.rx_buf = data;
	rx_param.rx_len = len;
	rx_param.tmo = timeout;
	rlt = ioctl(g_alislinput_panel_handle, ALI_FP_DATA_TRANSFER, (unsigned long)(&rx_param));
	if(0==rlt)
	{
		while(timeout--)
		{
			rlt = ioctl(g_alislinput_panel_handle, ALI_FP_TRANSFER_CHECK, 0);
			if(0==rlt)
			{
				rlt = rx_param.rx_len;
				break;
			}
			usleep(1 * 1000);
		}
	}

	return rlt;
}


long alislinput_buff_clear(struct alislinput_device *dev)
{
	struct input_event key_event;
	unsigned long max_cnt;
	long read_len = sizeof(struct input_event);


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n",
			g_alislinput_device_status);

		return -1;
	}

	if(NULL== dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	max_cnt = 1000;
	while(max_cnt--)
	{
		if (read_len != read(g_alislinput_input_handle_panel, &key_event, read_len))
		{
			break;
		}
	}
	max_cnt = 1000;
	while(max_cnt--)
	{
		if (read_len != read(g_alislinput_input_handle_ir, &key_event, read_len))
		{
			break;
		}
	}

	return 0;
}


void alislinput_buff_set_repeat(struct alislinput_device *dev, unsigned char enable_repeat)
{
	g_alislinput_repeat_enable = enable_repeat;
}


unsigned char alislinput_buff_get_repeat(struct alislinput_device *dev)
{
	return g_alislinput_repeat_enable;
}


long alislinput_config_ir_rep_interval(struct alislinput_device *dev, unsigned long delay, unsigned long interval)
{
	unsigned long rep[2] = {0, 0};
	long result = 0;

	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	rep[0] = delay;
	rep[1] = interval;

	SL_DBG("rep[0] = %ld, rep[1] = %ld\n", rep[0], rep[1]);

	result |= ioctl(g_alislinput_input_handle_ir, EVIOCSREP, rep);
	result |= ioctl(g_alislinput_ir_handle, ALI_FP_SET_REPEAT_INTERVAL, rep);


	return result;
}


long alislinput_config_pan_rep_interval(struct alislinput_device *dev, unsigned long delay, unsigned long interval)
{
	unsigned long rep[2] = {0, 0};
	long result = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if (NULL == dev)
	{
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	rep[0] = delay;
	rep[1] = interval;

	SL_DBG("rep[0] = %ld, rep[1] = %ld\n", rep[0], rep[1]);

	result |= ioctl(g_alislinput_input_handle_panel, EVIOCSREP, rep);
	result |= ioctl(g_alislinput_panel_handle, ALI_FP_SET_REPEAT_INTERVAL, rep);


	return result;
}


long alislinput_display(struct alislinput_device *dev, char *data, unsigned long len)
{
//	unsigned long backup_len = 0;


	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n",
			g_alislinput_device_status);

		return -1;
	}

	if (NULL == dev)
	{
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	if(len>sizeof(g_alislinput_display_backup))
	{
		memcpy(g_alislinput_display_backup, data, sizeof(g_alislinput_display_backup));
		g_alislinput_display_backup[sizeof(g_alislinput_display_backup) - 1] = 0;
	}
	else
	{
		strcpy(g_alislinput_display_backup, data);
	}


	return write(g_alislinput_panel_handle, data, len);
}


long alislinput_get_display(struct alislinput_device *dev, char *data, unsigned long * len)
{
	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}


	strcpy(data, g_alislinput_display_backup);

	return 0;
}


long alislinput_config_ir_format(struct alislinput_device *dev, unsigned long format)
{
	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n",
			g_alislinput_device_status);

		return -1;
	}

	if(NULL==dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	g_alislinput_ir_format = format;
	SL_DBG("g_alislinput_ir_format = 0x%08x\n", (unsigned int)g_alislinput_ir_format);

	return 0;
}


long alislinput_io_control(struct alislinput_device *dev, long cmd, unsigned long param)
{
	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL== dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	return ioctl(g_alislinput_panel_handle, cmd, param);
}


long alislinput_ir_io_control(struct alislinput_device *dev, long cmd, unsigned long param)
{
	if (PAN_DEV_OPEN != g_alislinput_device_status)
	{
		SL_ERR("status error, now = %d\n", 
			g_alislinput_device_status);

		return -1;
	}

	if(NULL== dev){
		SL_ERR("error:  Invalid parameters!\n");
		return -1;
	}

	return ioctl(g_alislinput_ir_handle, cmd, param);
}

long alislinput_callback_register(struct alislinput_device *dev,
		alislinput_callback callback)
{
	int ir_err = 0;
	int panel_err = 0;
	if (dev == NULL || dev != g_dev) {
		SL_ERR("alislinput_del_event input error %p %p\n", dev, g_dev);
		return 1;
	}
	if (g_ev_handle == NULL) {
		alislevent_open(&g_ev_handle);
	}
	if (g_ev_handle == NULL) {
		SL_ERR("alislinput_callback_register error alislevent_open fail\n");
		return 1;
	}

	if (callback != NULL) {
		if (2 != g_phy_code) { 
            // the g_alislinput_ir_key_map or g_alislinput_pannel_key_mapis not NULL
			g_ir_slev.cb = callback;
			g_panel_slev.cb = callback;
			SL_DBG("alislinput_add_event ir %p cb: %p\n", g_ir_slev.fd, callback);
			ir_err = alislevent_add(g_ev_handle, &g_ir_slev);
			panel_err = alislevent_add(g_ev_handle, &g_panel_slev);
		} else {
		    // the g_alislinput_ir_key_map and g_alislinput_pannel_key_map were set to NULL
		    // when first open ali input module. see in "aui_key_open"
			g_ir_slev_k.cb = callback;
			g_panel_slev_k.cb = callback;
			SL_DBG("alislinput_add_event ir_k %p cb: %p\n", g_ir_slev_k.fd, callback);
			ir_err = alislevent_add(g_ev_handle, &g_ir_slev_k);
			panel_err = alislevent_add(g_ev_handle, &g_panel_slev_k);
		}
	} else {
		if (2 != g_phy_code) {
			SL_DBG("alislinput_del_event ir\n");
			ir_err = alislevent_del(g_ev_handle, &g_ir_slev);
			panel_err = alislevent_del(g_ev_handle, &g_panel_slev);
			g_ir_slev.cb = NULL;
			g_panel_slev.cb = NULL;
		} else {
			SL_DBG("alislinput_del_event ir_k\n");
			ir_err = alislevent_del(g_ev_handle, &g_ir_slev_k);
			panel_err = alislevent_del(g_ev_handle, &g_panel_slev_k);
			g_ir_slev_k.cb = NULL;
			g_panel_slev_k.cb = NULL;
		}
	}
	if (ir_err || panel_err) {
		SL_ERR("alislevent add/del error\n");
		return 1;
	} else {
		return 0;
	}
}
