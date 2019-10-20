
#include <bits_op.h>
#include <flag_op.h>

alisl_retcode flag_wait_bit_test(struct flag *flg, uint32_t ptn,
				 int mod, uint32_t tmout)
{
	struct timespec ots;
	struct timespec nts;
	struct timespec ts;
	uint32_t ms;
	int err = -1;
	alisl_retcode ret;
	int oldtype;

	pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock, &flg->cm);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

	while (1) {
		pthread_mutex_lock(&flg->cm);

		if (bit_test(flg->flg, ptn, mod)) {
			pthread_mutex_unlock(&flg->cm);
			ret = 0;
			goto exit;
		}

		if (tmout == -1) {
			err = pthread_cond_wait(&flg->cond, &flg->cm);
			if (err) {
				pthread_mutex_unlock(&flg->cm);
				ret = ERROR_WAIT;
				goto exit;
			}

			pthread_mutex_unlock(&flg->cm);
		} else {
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += (tmout / 1000);
			ts.tv_nsec += (tmout % 1000) * 1000000;
			while (ts.tv_nsec >= 1000000000) {
				ts.tv_nsec -= 1000000000;
				ts.tv_sec ++;
			}

			clock_gettime(CLOCK_REALTIME, &ots);

			err = pthread_cond_timedwait(&flg->cond, &flg->cm, &ts);
			if (err) {
				pthread_mutex_unlock(&flg->cm);
				ret = ERROR_TMOUT;
				goto exit;
			}

			clock_gettime(CLOCK_REALTIME, &nts);
			ms = (nts.tv_sec  - ots.tv_sec) * 1000
				 + (nts.tv_nsec - ots.tv_nsec) / 1000000;

			if (tmout > ms)
				tmout -= ms;
			else
				tmout = 0;

			pthread_mutex_unlock(&flg->cm);
		}
	}

exit:
	pthread_setcanceltype(oldtype, NULL);
	pthread_cleanup_pop(0);

	return ret;
}

void flag_init(struct flag *flg, uint32_t ptn)
{
	pthread_mutex_init(&flg->cm, NULL);
	pthread_mutex_lock(&flg->cm);
	pthread_cond_init(&flg->cond, NULL);
	flg->flg = ptn;
	pthread_mutex_unlock(&flg->cm);
}
