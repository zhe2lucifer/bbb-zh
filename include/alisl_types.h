#ifndef _ALISL_TYPES_H_
#define _ALISL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


/**< display tvsystem */
enum dis_tvsys {
    DIS_TVSYS_PAL,               /* PAL TV system */
    DIS_TVSYS_NTSC,
    DIS_TVSYS_PAL_M,
    DIS_TVSYS_PAL_N,
    DIS_TVSYS_PAL_60,
    DIS_TVSYS_NTSC_443,
    DIS_TVSYS_SECAM,
    DIS_TVSYS_MAC,
    DIS_TVSYS_LINE_720_25,
    DIS_TVSYS_LINE_720_30,
    DIS_TVSYS_LINE_1080_25,
    DIS_TVSYS_LINE_1080_30,
    DIS_TVSYS_LINE_1080_50,
    DIS_TVSYS_LINE_1080_60,
    DIS_TVSYS_LINE_1080_24,
    DIS_TVSYS_LINE_1152_ASS,
    DIS_TVSYS_LINE_1080_ASS,
    DIS_TVSYS_PAL_NC,
    DIS_TVSYS_LINE_576P_50_VESA,
    DIS_TVSYS_LINE_720P_60_VESA,
    DIS_TVSYS_LINE_1080P_60_VESA,
    /*4k*2k*/
    DIS_TVSYS_LINE_4096X2160_24,
    DIS_TVSYS_LINE_3840X2160_24,
    DIS_TVSYS_LINE_3840X2160_25,
    DIS_TVSYS_LINE_3840X2160_30,
    DIS_TVSYS_LINE_3840X2160_50,
    DIS_TVSYS_LINE_3840X2160_60,
    DIS_TVSYS_LINE_4096X2160_25,
    DIS_TVSYS_LINE_4096X2160_30,
    DIS_TVSYS_LINE_4096X2160_50,
    DIS_TVSYS_LINE_4096X2160_60
};


// These values identify some different wakeup reasons about WDT.
// Set-top box started as a result of a reboot from the watch dog, 0x52454254 'REBT'
#define ALISL_PWR_WDT (ALI_WDT_REBOOT)
// Set-top box started as a result of a reboot requested, 0x424F4F54 'BOOT'
#define ALISL_PWR_REBOOT (0x424F4F54)
// Set-top box started as a result of wake up form standby mode, 0x57414B45 'WAKE'
#define ALISL_PWR_STANDBY (0x57414B45)


#ifdef __cplusplus
}
#endif

#endif    /* _ALISL_TYPES_H_ */
