#ifndef ALIUPGPACKTOOL_H
#define ALIUPGPACKTOOL_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "aliota.h"

#define TOOL_VER          "ALiUpgPackTool 1.0.6"

#define MAX_STRING_LEN    128
#define MAX_CFG_LINE      256
#define MAX_NAND_PART     40
#define MAX_NOR_PART     40


#define BYTES_READ_PERTIME  (4 * 1024 * 1024)

/* OTA package micro */

#define ALIGN_128KB      (128*1024)
#define ALIGN_64KB       (64*1024)
#define ALIGN_16B        (16)

#define NOR_PART_NAME       "sflash"
#define BOOT_ENV_PART_NAME  "bootenv"

#define UPGPKT_CONFIG_NAME  "upgpkt.conf"

/**************************************************
 * Share with alisl
 *************************************************/
#define OTA_TS_MODULE_ZISE       (0x100000) //spec fix
#define OTA_TS_MODULE_VALIE_ZISE (0xFE200)  //spec fix
#define OTA_DOWNLOAD_BUFFER_SIZE (10*OTA_TS_MODULE_ZISE) /* should be mulriple of OTA_TS_MODULE_ZISE */

#define MAX_PART_NAME_LEN 24
#define MAX_PART          64 //nor(1)+nand(20)

typedef struct alislupg_header_imageinfo {
    /** Part name */
    char image_name[MAX_PART_NAME_LEN];
    /** Part image offset in the upg package */
    int  image_offset;
    /** Part image size */
    int  image_size;
} alislupg_header_imageinfo_t;

typedef struct alislupg_header {
    int  upg_ver;
    /** package version, including all groups version.
      * the store method deside by user, such as:
      * package_ver = (bootver<<16 | upgfwver<<8 | mainfwver<<0)
      */
    int  package_ver;
    /** nor+nand part count */
    int  part_cnt;
    /* ota_down_buf_size is the buffer size(M) for stb temply stores the ota ts data in the RAM
     * The size is desided by the user
     */
    int  ota_down_buf_size;
    int  reserve2;
    int  reserve3;
    int  reserve4;
    alislupg_header_imageinfo_t image_info[MAX_PART];
} alislupg_header_t;
/**************************************************
 * Share with alisl end
 *************************************************/


struct FlashPartInfo
{
    bool partSelect;
    char partName[MAX_PART_NAME_LEN];
    int  partSize;
    char partFileName[MAX_STRING_LEN];
    int  partFileSize;
    int  partFileAlignSize;
};

struct NorPartInfo
{
    char norPartName[MAX_PART_NAME_LEN];
    char norPartFileName[MAX_STRING_LEN];
    int  norPartFileSize;
    int  norPartFileAlignSize;
};

enum LOGTYPE
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_DEBUG,
};

class ALiUpgPackTool : public QObject
{
    Q_OBJECT

public:
    explicit ALiUpgPackTool(QObject *parent = 0);
    ~ALiUpgPackTool();
    int loadPktCfg(void);
    void updatePartDisp(void);
	void updatePartDisp_nor(void);
	int parseNorCfg(QString);
    int parseNandCfg(QString);
    int check_partitions_conflict(void);
    void upgLog(int, QString);

    bool m_norPartSelect[MAX_NOR_PART];
	bool m_nandPartSelect[MAX_NAND_PART];

    QString m_norPath;
    QString m_imageDir;
    QStringList m_nand_images;
	QStringList m_nor_images;

    bool m_genOtaPktSelect;

    void threadGenerateUpgPkt_cli(void *arg);
    int inputValidCheck_cli(void);

    void start_pack();

private slots:

private:
    QString m_mainFwVer;
    QString m_upgFwVer;
    QString m_bootVer;

	int load_norFlash(QString directory);
    int load_nandFlash(QString directory);

    FlashPartInfo m_nandPartInfo[MAX_NAND_PART]; //store the nand partiton information
    NorPartInfo  m_norPartInfo; //store the nand partiton information
    int  m_nandPartCnt; //store the nand partiton count
    bool m_isPacking;
    char m_imgName[MAX_STRING_LEN];
    char m_otaImgName[MAX_STRING_LEN];
    char m_upgfw_parts_group[MAX_PART][MAX_PART_NAME_LEN];
    int  m_upgfw_parts_cnt;
    char m_mainfw_parts_group[MAX_PART][MAX_PART_NAME_LEN];
    int  m_mainfw_parts_cnt;
    char m_boot_parts_group[MAX_PART][MAX_PART_NAME_LEN];
    int  m_boot_parts_cnt;
    char m_unupgradable_parts_group[MAX_PART][MAX_PART_NAME_LEN];
    int  m_unupgradable_parts_cnt;
    char m_extra_unupgradable_files[MAX_PART][MAX_PART_NAME_LEN];
    int  m_extra_unupgradable_files_cnt;
    ota_user_param_t m_otaUserParam;

	int m_norPartCnt;
	FlashPartInfo m_norPartInfo_ext[MAX_NOR_PART];
	int m_totalPartSize;
	int m_totalFileSize;

};

#endif // ALIUPGPACKTOOL_H
