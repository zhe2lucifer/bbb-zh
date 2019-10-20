/**
 * @brief		New a thread and use epoll monitoring multiple file descriptors
 * 				to see if I/O is possible on any of them.
 * @author		oscar.shi
 * @date		2015-7-8
 * @version		1.0.0
 * @note		ALi corp. all rights reserved. 2013~2920 copyright (C)
 */

#ifndef ALISLEVENT_H_
#define ALISLEVENT_H_

#include <alipltfretcode.h>

typedef void *(* alisl_event_callback)(void *);

/*! \struct alislevent
 */
struct alislevent
{
	/*! The file description */
	int fd;
	/*! Epoll events. See struct epoll_event member events.*/
	int events;
	/*! call back funtion */
	alisl_event_callback cb;
	/*! User data variable */
	void *data;
};

/*!
 * Open the alislevent module, get the handle.
 * @param[in,out] phandle The alislevent handle pointer, the open handle will
 *                        put in the address.
 * @return return 0 if open success.
 */
alisl_retcode alislevent_open(alisl_handle *phandle);

/*!
 * close the alislevent module.
 * @param[in] handle The alislevent handle.
 * @return return 0 if close success.
 */
alisl_retcode alislevent_close(alisl_handle handle);

/*!
 * Add fd to be poll.
 * @param[in] handle The alislevent handle.
 * @param[in] slev The struct alislevent to be poll.
 * @return return 0 if add success.
 * @note Make sure the data which slev point to is available.
 *       It's lifetime should be the same as handle.
 */
alisl_retcode alislevent_add(alisl_handle handle, struct alislevent *slev);

/*!
 * Stop poll fd.
 * @param[in] handle The alislevent handle.
 * @param[in] slev The struct alislevent to be remove.
 * @return return 0 if delete success.
 * @note Make sure the data which slev point to is available.
 *       It's lifetime should be the same as handle.
 */
alisl_retcode alislevent_del(alisl_handle handle, struct alislevent *slev);

/*!
 * Modify the event event associated with the target file.
 * @param[in] handle The alislevent handle.
 * @param[in] slev The struct alislevent.
 * @return return 0 if modify success.
 * @note Make sure the data which slev point to is available.
 *       It's lifetime should be the same as handle.
 */
alisl_retcode alislevent_mod(alisl_handle handle, struct alislevent *slev);

#endif /* ALISLEVENT_H_ */
