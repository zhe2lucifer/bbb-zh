#ifndef __ALISLVMXPLUS__H_
#define __ALISLVMXPLUS__H_

#include <alipltfretcode.h>
//#include <ali_dsc_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define VMX_SERVICE_NAME_DVB0       "/dev/ali_bc_dvb0"
#define VMX_SERVICE_NAME_DVB1       "/dev/ali_bc_dvb1"
#define VMX_SERVICE_NAME_DVB2       "/dev/ali_bc_dvb2"
#define VMX_SERVICE_NAME_IPTV0      "/dev/ali_bc_iptv0"
#define VMX_SERVICE_NAME_IPTV1      "/dev/ali_bc_iptv1"
#define VMX_SERVICE_NAME_IPTV2      "/dev/ali_bc_iptv2"
#define VMX_SERVICE_NAME_DVR0       "/dev/ali_bc_dvr0"
#define VMX_SERVICE_NAME_DVR1       "/dev/ali_bc_dvr1"
#define VMX_SERVICE_NAME_DVR2       "/dev/ali_bc_dvr2"
#define VMX_SERVICE_NAME_OTT0       "/dev/ali_bc_ott0"
#define VMX_SERVICE_NAME_OTT1       "/dev/ali_bc_ott1"

/*
#define VMX_SERVICE_DVB0        0x00
#define VMX_SERVICE_DVB1        0x01
#define VMX_SERVICE_DVB2        0x02
#define VMX_SERVICE_IPTV0       0x40
#define VMX_SERVICE_IPTV1       0x41
#define VMX_SERVICE_IPTV2       0x42
#define VMX_SERVICE_DVR0        0x80
#define VMX_SERVICE_DVR1        0x81
#define VMX_SERVICE_DVR2        0x82
#define VMX_SERVICE_OTT0        0xC0
#define VMX_SERVICE_OTT1        0xC1
*/
typedef enum{
    VMX_SERVICE_DVB0 = 0,
    VMX_SERVICE_DVB1,
    VMX_SERVICE_DVB2,
    VMX_SERVICE_IPTV0,
    VMX_SERVICE_IPTV1,
    VMX_SERVICE_IPTV2,
    VMX_SERVICE_DVR0,
    VMX_SERVICE_DVR1,
    VMX_SERVICE_DVR2,
    VMX_SERVICE_OTT0,
    VMX_SERVICE_OTT1
}vmx_service_index;

typedef enum{
    VMX_IO_SET_CA_ALOG = 0,
    VMX_IO_SET_PID,
    VMX_IO_GET_DSC_STATUS,
    VMX_IO_GET_KUMSGQ,
    VMX_IO_RPC_RET
}vmx_io_cmd;

alisl_retcode alisl_vmx_open(vmx_service_index service_index, alisl_handle *handle);
alisl_retcode alisl_vmx_close(alisl_handle handle);
int alisl_vmx_ioctl(alisl_handle handle, vmx_io_cmd cmd, void *param);
alisl_retcode alisl_vmx_fd_get(alisl_handle handle, int *vmx_fd);

#ifdef __cplusplus
}
#endif


#endif //end of __ALISLVMXPLUS__H_
