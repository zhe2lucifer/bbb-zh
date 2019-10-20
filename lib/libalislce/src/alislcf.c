#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include <alipltflog.h>
#include <alipltfretcode.h>

#include "ce_internal.h"
#include "ca_dsc.h"
#include "ca_kl.h"
#include "errno.h"
#include "alislce.h"
#include "ali_cf.h"

alisl_retcode alislcf_set_target_pos(alisl_handle handle, enum sl_ce_key_parity sl_ce_parity)
{
    alisl_retcode ret = 0;
    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
	struct ali_cf_cwc_target cf_target;

    if (NULL == ce_dev) {
        SL_DBG("Invalid parameters!\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    //pthread_mutex_lock(&m_mutex);
	memset(&cf_target,0,sizeof(cf_target));
	if (ce_dev->target_fd < 0){
	    ce_dev->target_fd = open(ALISLCF_DEV_NAME, O_RDWR);;
	    if (ce_dev->fd < 0) {

	        SL_DBG("open cf failed\n");
		return ALISLCE_ERR_IOACCESS;
	    }
	}
	SL_DBG("open %s success,target_fd = %d\n",ALISLCF_DEV_NAME,ce_dev->target_fd);
	cf_target.fd = ce_dev->fd;
	if (sl_ce_parity == SL_CE_KEY_PARITY_ODD){
		cf_target.parity = CF_PARITY_ODD;
		ce_dev->target_parity = CF_PARITY_ODD;
	}else if (sl_ce_parity == SL_CE_KEY_PARITY_EVEN){
		cf_target.parity = CF_PARITY_EVEN;
		ce_dev->target_parity = CF_PARITY_EVEN;
	}else{
		return ALISLCE_ERR_INVALIDPARAM;
	}
	SL_DBG("cf_target.fd: %d,cf_target.parity: %d\n",cf_target.fd,cf_target.parity);
	ret |= ioctl(ce_dev->target_fd,ALI_CF_IOC_SET_CWC_TARGET,&cf_target);
	if (ret < 0){
		SL_DBG("set cf key target pos error\n");
		return ALISLCE_ERR_IOACCESS;
	}
    //pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alislcf_get_target_dev_param(alisl_handle handle, 
	struct kl_cw_derivation *key_config)
{
//	alisl_retcode ret = 0;
    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
//	struct ali_cf_cwc_target cf_target;

    if (NULL == ce_dev) {
        SL_DBG("Invalid parameters!\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

	key_config->target_fd = ce_dev->fd;
	//key_config->target_parity = ce_dev->target_parity;
	SL_DBG("kl_fd: %d,key_config->target_fd: %d,key_config->target_parity: %d\n",
		ce_dev->fd,key_config->target_fd,key_config->target_parity);
	return 0;
}
