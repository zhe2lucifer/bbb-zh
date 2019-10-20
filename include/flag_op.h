#ifndef __ALILS_FLAG_H__
#define __ALILS_FLAG_H__

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <alipltfretcode.h>

struct flag {
	uint32_t                flg;            /**< current flag value */
	pthread_mutex_t         cm;             /**< mutex, used for cond */
	pthread_cond_t          cond;
};

void flag_init(struct flag *flg, uint32_t ptn);

alisl_retcode flag_wait_bit_test(struct flag *flg, uint32_t ptn,
				 int mod, uint32_t tmout);

static inline alisl_retcode flag_wait_bit_any(struct flag *flg, uint32_t ptn, uint32_t tmout)
{
	return flag_wait_bit_test(flg, ptn, TEST_MODE_ANY, tmout);
}

static inline alisl_retcode flag_wait_bit_all(struct flag *flg, uint32_t ptn, uint32_t tmout)
{
	return flag_wait_bit_test(flg, ptn, TEST_MODE_ALL, tmout);
}

static inline alisl_retcode flag_wait_bit_eq(struct flag *flg, uint32_t ptn, uint32_t tmout)
{
	return flag_wait_bit_test(flg, ptn, TEST_MODE_EQ, tmout);
}

static inline bool flag_bit_test_any(struct flag *flg, uint32_t ptn)
{
	bool ret;
	pthread_mutex_lock(&flg->cm);
	ret = bit_test_any(flg->flg, ptn);
	pthread_mutex_unlock(&flg->cm);

	return ret;
}

static inline int flag_bit_set(struct flag *flg, uint32_t ptn)
{
    pthread_mutex_lock(&flg->cm);
    bit_set(flg->flg, ptn);
    pthread_cond_broadcast(&flg->cond);
    pthread_mutex_unlock(&flg->cm);

    return 0;
}

static inline int flag_bit_clear(struct flag *flg, uint32_t ptn)
{
    pthread_mutex_lock(&flg->cm);
    bit_clear(flg->flg, ptn);
    pthread_mutex_unlock(&flg->cm);

    return 0;
}

static inline int flag_delete(struct flag *flg)
{
    pthread_mutex_destroy(&flg->cm);
    pthread_cond_destroy(&flg->cond);
    return 0;
}

#endif
