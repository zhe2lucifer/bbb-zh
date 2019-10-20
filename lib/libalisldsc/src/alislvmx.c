#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include <alisldsc.h>
#include "dsc_internal.h"
#include "alislvmx.h"

#include "linux/ali_sec.h"
 
typedef struct alisl_vmx_dev{
    alisldsc_dev_t dsc_dev;
    unsigned char service_index;
    pthread_mutex_t vmx_mutex;
}alisl_vmx_dev;


//#define VMX_SL_PRINT_DEBUG(fmt, ...)   do{}while(0)

alisl_retcode alisl_vmx_open(vmx_service_index service_index, alisl_handle *handle)
{
    alisl_vmx_dev *vmx_dev = NULL;
    char dev_name[32];
    
    SL_DBG("service_index=%d\n", (int)service_index);
    switch (service_index){
        case VMX_SERVICE_DVB0:
            strcpy(dev_name, VMX_SERVICE_NAME_DVB0);
            break;
        case VMX_SERVICE_DVB1:
            strcpy(dev_name, VMX_SERVICE_NAME_DVB1);
            break;
        case VMX_SERVICE_DVB2:
            strcpy(dev_name, VMX_SERVICE_NAME_DVB2);
            break;
        case VMX_SERVICE_IPTV0:
            strcpy(dev_name, VMX_SERVICE_NAME_IPTV0);
            break;
        case VMX_SERVICE_IPTV1:
            strcpy(dev_name, VMX_SERVICE_NAME_IPTV1);
            break;
        case VMX_SERVICE_IPTV2:
            strcpy(dev_name, VMX_SERVICE_NAME_IPTV2);
            break;
        case VMX_SERVICE_DVR0:
            strcpy(dev_name, VMX_SERVICE_NAME_DVR0);
            break;
        case VMX_SERVICE_DVR1:
            strcpy(dev_name, VMX_SERVICE_NAME_DVR1);
            break;
        case VMX_SERVICE_DVR2:
            strcpy(dev_name, VMX_SERVICE_NAME_DVR2);
            break;
        case VMX_SERVICE_OTT0:
            strcpy(dev_name, VMX_SERVICE_NAME_OTT0);
            break;
        case VMX_SERVICE_OTT1:
            strcpy(dev_name, VMX_SERVICE_NAME_OTT1);
            break;
        default:
            SL_ERR("service index is wrong!\n");
            return ALISLDSC_ERR_INVALIDPARAM;
    }

    vmx_dev = (alisl_vmx_dev*)calloc(sizeof(alisl_vmx_dev), 1);
    if (NULL == vmx_dev){
        SL_ERR("malloc vmx device error!\n");
        return ALISLDSC_ERR_MALLOCFAIL;
    }

    pthread_mutex_init(&vmx_dev->vmx_mutex, NULL);

    pthread_mutex_lock(&vmx_dev->vmx_mutex);

    vmx_dev->service_index = service_index;
    vmx_dev->dsc_dev.fd = open(dev_name, O_RDWR);
    if (vmx_dev->dsc_dev.fd < 0){
        SL_ERR("can not open vmx device :%s!\n", dev_name);
        pthread_mutex_unlock(&vmx_dev->vmx_mutex);
		free(vmx_dev);
        return ALISLDSC_ERR_CANTOPENDEV;        
    }
    
    *handle = (alisl_handle *)vmx_dev;
    pthread_mutex_unlock(&vmx_dev->vmx_mutex);

    SL_DBG("open <%s>: dev_fd=%d\n", dev_name, vmx_dev->dsc_dev.fd);
    return 0;
}

alisl_retcode alisl_vmx_close(alisl_handle handle)
{
    alisl_vmx_dev *vmx_dev = (alisl_vmx_dev *)handle;
    int ret;
    
    if(NULL == handle) {
        SL_ERR("vmx_dev is NULL!\n");
        return ALISLDSC_ERR_INVALIDHANDLE;
    }
    SL_DBG("close fd: %d\n", vmx_dev->dsc_dev.fd);
    pthread_mutex_lock(&vmx_dev->vmx_mutex);

    ret = close(vmx_dev->dsc_dev.fd);
    if (ret){
        SL_ERR("close vmx error!\n");
        pthread_mutex_unlock(&vmx_dev->vmx_mutex);
        return ALISLDSC_ERR_CANTOPENDEV; 
    }

    pthread_mutex_unlock(&vmx_dev->vmx_mutex);
    pthread_mutex_destroy(&vmx_dev->vmx_mutex);

    free(vmx_dev);
    return 0;
}

int alisl_vmx_ioctl(alisl_handle handle, vmx_io_cmd cmd, void *param)
{
    int ret = 0;
    unsigned long io_cmd;
    alisl_vmx_dev *vmx_dev = (alisl_vmx_dev *)handle;

    if(NULL == handle) {
        SL_ERR("vmx_dev is NULL!\n");
        return ALISLDSC_ERR_INVALIDHANDLE;
    }
    switch (cmd){
    case VMX_IO_SET_CA_ALOG:
        io_cmd = IO_SEC_SET_BC_CA_ALGO;
        break;
    case VMX_IO_SET_PID:
        io_cmd = IO_SEC_SET_BC_DVR_PID;
        break;
    case VMX_IO_GET_DSC_STATUS:
        io_cmd = IO_SEC_GET_BC_DSC_STATUS;
        break;
    case VMX_IO_GET_KUMSGQ:
        io_cmd = IO_SEC_GET_KUMSGQ;
        break;
    case VMX_IO_RPC_RET:
        io_cmd = IO_SEC_RPC_RET;
        break;
    default:
        SL_ERR("IO cmd is error: <%d>\n", (int)cmd);
        return -1;
    }
    
    pthread_mutex_lock(&vmx_dev->vmx_mutex);
    ret = ioctl(vmx_dev->dsc_dev.fd, io_cmd, param);
    pthread_mutex_unlock(&vmx_dev->vmx_mutex);

    SL_DBG("vmx device ioctl cmd<0x%x>, io_cmd<0x%x> return: %d! fd: %d\n", 
        (unsigned int)cmd, (unsigned int)io_cmd, ret, vmx_dev->dsc_dev.fd);

    return ret;
    
}


alisl_retcode alisl_vmx_fd_get(alisl_handle handle, int *vmx_fd)
{
    alisl_retcode ret = 0;
    alisl_vmx_dev *vmx_dev = (alisl_vmx_dev *)handle;
    
    *vmx_fd = vmx_dev->dsc_dev.fd;
   	SL_DBG("get vmx fd: %d\n",*vmx_fd);
    return ret;
}

