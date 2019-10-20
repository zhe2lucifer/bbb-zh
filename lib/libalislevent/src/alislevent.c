/*
 * alislevent.c
 *
 *  Created on: 2015.07.06
 *      Author: osc
 */

#include <alislevent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <alipltflog.h>

#define MX_EVNTS 10
#define EPL_TOUT 3000

typedef struct alislevent_private
{
	pthread_t tid;
	int eplfd;
	int open_cnt;
	int running;
} alislevent_private_t;

static alislevent_private_t m_alislevent;
static pthread_mutex_t m_alislevent_mutex = PTHREAD_MUTEX_INITIALIZER;

void *alislevent_func(void *arg)
{
	struct epoll_event evnts[MX_EVNTS];
	int *eplfd = (int *)arg;
	struct alislevent slev;
	int n = -1;
	int i;

	// Set thread detached to avoid block some time in pthread_join
	pthread_detach(pthread_self());

	while (1) {
		pthread_mutex_lock(&m_alislevent_mutex);
		if (!m_alislevent.running) {
			pthread_mutex_unlock(&m_alislevent_mutex);
			break;
		}
		pthread_mutex_unlock(&m_alislevent_mutex);
		n = epoll_wait(*eplfd, evnts, MX_EVNTS, EPL_TOUT);
		if (n == -1) {
			//should ignore EINTR error resulting from some exception, changing user for example 
			if(EINTR == errno) {
				//SL_ERR("EINTR \n");
				continue;
			}
			//SL_ERR("epoll_wait error");
			break;
		} else if (n == 0) {
//			SL_DBG("time out %d sec expired\n",EPL_TOUT / 1000);
			continue;
		}
//		SL_DBG("%d files is ready!\n", n);
		for(i = 0; i < n; i++){
			slev = *(struct alislevent *)evnts[i].data.ptr;
//			SL_DBG("evnts[i].data.ptr: %p\n", evnts[i].data.ptr);
//			SL_DBG("alislevent fd: %x\n", slev.fd);
//			SL_DBG("alislevent events: %x\n", slev.events);
//			SL_DBG("alislevent cb: %p\n", slev.cb);
//			SL_DBG("alislevent data: %p\n", slev.data);
			slev.events = evnts[i].events;

			// Check the running flag again
			pthread_mutex_lock(&m_alislevent_mutex);
			if (!m_alislevent.running) {
				pthread_mutex_unlock(&m_alislevent_mutex);
				break;
			}
			pthread_mutex_unlock(&m_alislevent_mutex);
			// The upper layer may do many thing in the call back,
			// so some callback may block a long time.
			slev.cb(slev.data);
		}
	}
	/* Don`t put close(fd) here, because once calling pthread_cancel this thread,
	this thread function would exit anywhere, close(fd) would not executed 
	and happen resource leak. You can man pthread_cancel for more detailed information. 
	*/
	//close(*eplfd);
	pthread_exit(NULL);
}

alisl_retcode alislevent_open(alisl_handle *phandle)
{
	if (phandle == NULL) {
		SL_ERR("Input data error");
		return -1;
	}
	pthread_mutex_lock(&m_alislevent_mutex);
	if (m_alislevent.open_cnt < 1) {
		m_alislevent.eplfd = epoll_create1(0);
		if (m_alislevent.eplfd < 0){
			SL_ERR("epoll_create1 error");
			pthread_mutex_unlock(&m_alislevent_mutex);
			return -1;
		}
		m_alislevent.open_cnt = 0;
	}
	m_alislevent.open_cnt++;
	*phandle = &m_alislevent;
	pthread_mutex_unlock(&m_alislevent_mutex);
	return 0;
}

alisl_retcode alislevent_close(alisl_handle handle)
{
	if (handle == NULL || handle != &m_alislevent) {
		SL_ERR("Input data error");
		return -1;
	}
	pthread_mutex_lock(&m_alislevent_mutex);
	if (m_alislevent.open_cnt > 0) {
		m_alislevent.open_cnt--;
	} else {
		//SL_ERR("alislevent is not open\n");
		pthread_mutex_unlock(&m_alislevent_mutex);
		return -1;
	}
	if (m_alislevent.open_cnt) {
		pthread_mutex_unlock(&m_alislevent_mutex);
		return 0;
	}
	if (m_alislevent.running) {
		m_alislevent.running = 0;
// Marks thread detached by pthread_detach to avoid block in pthread_join
//		//pthread_cancel(m_alislevent.tid);
//		if(pthread_join(m_alislevent.tid, NULL) != 0){
//			//SL_ERR("pthread_join error");
//			pthread_mutex_unlock(&m_alislevent_mutex);
//			return -1;
//		}
	}
	if (m_alislevent.eplfd>=0) {
		close(m_alislevent.eplfd);
		m_alislevent.eplfd = -1;
	}
	pthread_mutex_unlock(&m_alislevent_mutex);
	return 0;
}

alisl_retcode alislevent_add(alisl_handle handle, struct alislevent *slev)
{
	struct epoll_event ev;

	if (handle == NULL || handle != &m_alislevent || slev == NULL) {
		SL_ERR("Input data error");
		return -1;
	}

	ev.events = slev->events;
	ev.data.ptr = slev;
	if (epoll_ctl(m_alislevent.eplfd, EPOLL_CTL_ADD, slev->fd, &ev) != 0){
		SL_ERR("epoll_ctl EPOLL_CTL_ADD error");
		return -1;
	}

	pthread_mutex_lock(&m_alislevent_mutex);
	if (m_alislevent.running) {
		pthread_mutex_unlock(&m_alislevent_mutex);
		return 0;
	}

	if (pthread_create(&m_alislevent.tid, NULL, alislevent_func,
			(void *)&m_alislevent.eplfd)) {
		SL_ERR("pthread_create  error");
		if (epoll_ctl(m_alislevent.eplfd, EPOLL_CTL_DEL, slev->fd, NULL)) {
			SL_ERR("epoll_ctl EPOLL_CTL_DEL error");
		}
		pthread_mutex_unlock(&m_alislevent_mutex);
		return -1;
	} else {
		m_alislevent.running = 1;
	}
	pthread_mutex_unlock(&m_alislevent_mutex);
	return 0;
}

alisl_retcode alislevent_del(alisl_handle handle, struct alislevent *slev)
{
	if (handle == NULL || handle != &m_alislevent || slev == NULL) {
		SL_ERR("Input data error");
		return -1;
	}
	if (epoll_ctl(m_alislevent.eplfd, EPOLL_CTL_DEL, slev->fd, NULL)){
		//SL_ERR("epoll_ctl EPOLL_CTL_DEL error");
		SL_ERR("EPOLL_CTL_DEL fail.slev: %p fd: %d\n", slev, slev->fd);
		return -1;
	}
	return 0;
}

alisl_retcode alislevent_mod(alisl_handle handle, struct alislevent *slev)
{
	struct epoll_event ev;

	if (handle == NULL || handle != &m_alislevent || slev == NULL) {
		SL_ERR("Input data error");
		return -1;
	}
	ev.events = slev->events;
	ev.data.ptr = slev;
	if (epoll_ctl(m_alislevent.eplfd, EPOLL_CTL_MOD, slev->fd, &ev)){
		SL_ERR("epoll_ctl EPOLL_CTL_MOD error");
		return -1;
	}
	return 0;
}

