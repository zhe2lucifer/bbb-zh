#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iosfwd>
#include <fstream>

#include "fastCRC.h"
#include "aliupgpacktool.h"
#include "aliota.h"

using namespace std;

enum ERROR_CODE
{
    SUCCESS,
    ERROR_PART_NOT_SELECTED,
    ERROR_VERSION_NOT_INPUT,
    ERROR_PART_CONFLICT,
    ERROR_OTHER,
};


//QString to Char, such as: QString str("hello") -> ch[MAX_STRING_LEN]="hello". (max_len=MAX_STRING_LEN)
static char* qstringToChar(QString qstring, char* ch)
{
    if(NULL == ch)
    {
        return NULL;
    }

    QByteArray ba = qstring.toLatin1();
    if(ba.length()<MAX_STRING_LEN)
    {
        strcpy(ch, ba.data());
    }
    else
    {
        return NULL;
    }

    return ch;
}

//QString hex to int, such as: QString str("0x123") -> int a=291. (max_len=MAX_STRING_LEN)
static int qstringHexToInt(QString qstring, int* desc)
{
    char temp[MAX_STRING_LEN];

    memset(temp, 0, MAX_STRING_LEN);
    qstringToChar(qstring, temp);
    *desc = strtol(temp, NULL, 16);

    return *desc;
}

//QString Decimal to int, such as: QString str("123") -> int a=123. (max_len=MAX_STRING_LEN)
static int qstringDecToInt(QString qstring, int* desc)
{
    char temp[MAX_STRING_LEN];

    memset(temp, 0, MAX_STRING_LEN);
    qstringToChar(qstring, temp);
    *desc = atoi(temp);

    return *desc;
}

static int align(int size_align, int size)
{
    int retSize = 0;
    if(0 == ((size) % (size_align)))
    {
        retSize = size;
    }
    else
    {
        retSize = (size) + (size_align - ((size) % (size_align)));
    }
    return retSize;
}


/* call the C library funtion to split the string */
static char *_strsep(char **stringp, const char *delim)
{
    register char *tmp = *stringp;
    register char *tmp2 = tmp;
    register const char *tmp3;

    if (!*stringp) return 0;

    for (tmp2 = tmp; *tmp2; ++tmp2)
    {
        for (tmp3 = delim; *tmp3; ++tmp3)
        {
            if (*tmp2 == *tmp3)
            {
                /* delimiter found */
                *tmp2 = 0;
                *stringp = tmp2 + 1;
                return tmp;
            }
        }
    }

    *stringp = 0;
    return tmp;
}

static int transform_line_to_group(char *line_buf, char group[][MAX_PART_NAME_LEN])
{
    int i = 0;

    char *token = _strsep(&line_buf, "|");

    while (token)
    {
        strcpy(group[i++], token);
        token = _strsep(&line_buf, "|");
    }

    return i;
}

ALiUpgPackTool::ALiUpgPackTool(QObject *parent) :
    QObject(parent)
{
    m_mainFwVer = "0";
    m_upgFwVer = "0";
    m_bootVer = "0";
    m_genOtaPktSelect =false;

    //parse upgpkg.conf
    loadPktCfg();

    load_nandFlash(m_imageDir);
    load_norFlash(m_imageDir);

    m_isPacking = false;
 
//    start_pack();
}

ALiUpgPackTool::~ALiUpgPackTool()
{
}

int ALiUpgPackTool::loadPktCfg(void)
{
    QString strLog;
    QString path;
    char line_buf[(MAX_PART)*MAX_PART_NAME_LEN];

    path.sprintf(UPGPKT_CONFIG_NAME);

    QFileInfo nandCfgFileInfo(path);
    if (!nandCfgFileInfo.exists())
    {
        strLog = QString("open pkt config file fail: %1").arg(path);
        upgLog(LOG_ERROR, strLog);
        return -1;
    }
    strLog = QString("pkt config file: %1").arg(path);
    upgLog(LOG_DEBUG, strLog);

    QSettings settings(path, QSettings::IniFormat);

    //image_name
    qstringToChar(settings.value("OUTPUT_SETTING/IMAGE_NAME").toString(), m_imgName);
    //ota_image_name
    qstringToChar(settings.value("OUTPUT_SETTING/OTA_IMAGE_NAME").toString(), m_otaImgName);

    //partitions group
    memset(line_buf, 0, sizeof(line_buf));
    qstringToChar(settings.value("PARTITION_GROUPS/MAINFW_GROUP").toString(), line_buf);
    m_mainfw_parts_cnt = transform_line_to_group(line_buf, m_mainfw_parts_group);
    memset(line_buf, 0, sizeof(line_buf));
    qstringToChar(settings.value("PARTITION_GROUPS/UPGFW_GROUP").toString(), line_buf);
    m_upgfw_parts_cnt = transform_line_to_group(line_buf, m_upgfw_parts_group);
    memset(line_buf, 0, sizeof(line_buf));
    qstringToChar(settings.value("PARTITION_GROUPS/BOOT_GROUP").toString(), line_buf);
    m_boot_parts_cnt = transform_line_to_group(line_buf, m_boot_parts_group);
    memset(line_buf, 0, sizeof(line_buf));
    qstringToChar(settings.value("PARTITION_GROUPS/UNUPGRADABLE_GROUP").toString(), line_buf);
    m_unupgradable_parts_cnt = transform_line_to_group(line_buf, m_unupgradable_parts_group);

    //extra setting
    memset(line_buf, 0, sizeof(line_buf));
    qstringToChar(settings.value("EXTRA_SETTING/EXTRA_UNUPGRADABLE_FILES").toString(), line_buf);
    m_extra_unupgradable_files_cnt = transform_line_to_group(line_buf, m_extra_unupgradable_files);

    memset(&m_otaUserParam, 0, sizeof(m_otaUserParam));
    //oui
    qstringHexToInt(settings.value("OTA_BASIC_SETTING/OUI").toString(), &m_otaUserParam.oui);
    //hw_model
    qstringHexToInt(settings.value("OTA_BASIC_SETTING/HW_MODEL").toString(), &m_otaUserParam.hw_model);
    //hw_ver
    qstringHexToInt(settings.value("OTA_BASIC_SETTING/HW_VER").toString(), &m_otaUserParam.hw_ver);
    //sw_model
    qstringHexToInt(settings.value("OTA_BASIC_SETTING/SW_MODEL").toString(), &m_otaUserParam.sw_model);
    //sw_ver
    qstringHexToInt(settings.value("OTA_BASIC_SETTING/SW_VER").toString(), &m_otaUserParam.sw_ver);

    //ota_buffer_size
    m_otaUserParam.down_buffer_size = settings.value("OTA_ADVANCE_SETTING/OTA_DOWN_BUFFER_SIZE").toInt();
    //pid
    qstringHexToInt(settings.value("OTA_ADVANCE_SETTING/OTA_PID").toString(), &m_otaUserParam.pid);
    //insert_table
    m_otaUserParam.insert_table = settings.value("OTA_ADVANCE_SETTING/OTA_INSERT_TABLE").toInt();
    //ts_id
    qstringHexToInt(settings.value("OTA_ADVANCE_SETTING/OTA_TS_ID").toString(), &m_otaUserParam.ts_id);
    //pmt_pid
    qstringHexToInt(settings.value("OTA_ADVANCE_SETTING/OTA_PMT_PID").toString(), &m_otaUserParam.pmt_pid);
    //prog_num
    qstringHexToInt(settings.value("OTA_ADVANCE_SETTING/OTA_PROG_NUM").toString(), &m_otaUserParam.prog_num);
    //service_name
    qstringToChar(settings.value("OTA_ADVANCE_SETTING/OTA_SERVICE_NAME").toString(), m_otaUserParam.service_name);
    //service_provider
    qstringToChar(settings.value("OTA_ADVANCE_SETTING/OTA_SERVICE_PROVIDER").toString(), m_otaUserParam.service_provider);

    m_imageDir = settings.value("IMAGE/IMAGE_PATH").toString();
    if (m_imageDir.endsWith('/'))
    {
        m_imageDir.resize(m_imageDir.size() - 1);
    }

    m_norPath = settings.value("IMAGE/NOR_IMAGE").toString();
    /*if (!m_norPath.isEmpty())
    {
        if (!m_imageDir.isEmpty())
        {
            m_norPath.push_front("/");
        }
        m_norPath.push_front(m_imageDir);
    }*/

    m_nand_images = settings.value("IMAGE/NAND_IMAGE").toStringList();

    m_nor_images = settings.value("IMAGE/NOR_IMAGE").toStringList();

    qDebug() << "nand dir: " << m_imageDir << "\nnor path: " << m_norPath \
        << "\nnand images: " <<m_nand_images << "\nnor images: " << m_nor_images << endl;

    m_mainFwVer = settings.value("IMAGE/MAIN_FW_VER").toString();
    m_upgFwVer = settings.value("IMAGE/UPG_FW_VER").toString();
    m_bootVer = settings.value("IMAGE/BOOT_VER").toString();

    m_genOtaPktSelect = settings.value("IMAGE/GEN_OTA_PKT", false).toBool();

    m_totalFileSize = 0;
    m_totalPartSize = 0;
    return 0;
}

void ALiUpgPackTool::updatePartDisp()
{
    int  i = 0;
    int  j = 0;
    bool upgradable = true;

    for(i=0; i<m_nandPartCnt; i++)
    {
        upgradable = true;
        for(j=0; j<m_unupgradable_parts_cnt; j++)
        {
            if(0 == strcmp(m_unupgradable_parts_group[j], m_nandPartInfo[i].partName))
            {
                upgradable = false;
                break;
            }
        }
        for(j=0; j<m_extra_unupgradable_files_cnt; j++)
        {
            if(0 == strcmp(m_extra_unupgradable_files[j], m_nandPartInfo[i].partFileName))
            {
                upgradable = false;
                break;
            }
        }
        if(!upgradable)
        {
            m_nandPartSelect[i] = false;
        }
        else
        {
            m_nandPartSelect[i] = true;
        }

        //qDebug("updatePartDisp, i: %d, select: %d", i, m_nandPartSelect[i]);
    }

    for(i=m_nandPartCnt; i<MAX_NAND_PART; i++)
    {
        m_nandPartSelect[i] = false;
    }

	
}

void ALiUpgPackTool::updatePartDisp_nor()
{
    int  i = 0;
    int  j = 0;
    bool upgradable = true;

    for(i=0; i<m_norPartCnt; i++)
    {
        upgradable = true;
        for(j=0; j<m_unupgradable_parts_cnt; j++)
        {
            if(0 == strcmp(m_unupgradable_parts_group[j], m_norPartInfo_ext[i].partName))
            {
                upgradable = false;
                break;
            }
        }
        for(j=0; j<m_extra_unupgradable_files_cnt; j++)
        {
            if(0 == strcmp(m_extra_unupgradable_files[j], m_norPartInfo_ext[i].partFileName))
            {
                upgradable = false;
                break;
            }
        }
        if(!upgradable)
        {
            m_norPartSelect[i] = false;
        }
        else
        {
            m_norPartSelect[i] = true;
        }
        //qDebug("updatePartDisp_nor, i: %d, select: %d", i, m_norPartSelect[i]);
    }


    for(i=m_norPartCnt; i<MAX_NOR_PART; i++)
    {
        m_norPartSelect[i] = false;
    }

}


int ALiUpgPackTool::parseNorCfg(QString path)
{
    int i = 0;
    QString strTemp, strIni, strLog;
    ifstream norPartFileName;
    int  partFileLen = 0;
    char temp[MAX_STRING_LEN];
    QString loader;

    QFileInfo norCfgFileInfo(path);
    if (!norCfgFileInfo.exists())
    {
        strLog = QString("parseNorCfg, open file fail: %1").arg(path);
        upgLog(LOG_ERROR, strLog);
        return -1;
    }
    strLog = QString("parseNorCfg, config file: %1").arg(path);
    upgLog(LOG_DEBUG, strLog);

    QSettings settings(path, QSettings::IniFormat);

    m_norPartCnt = settings.value("NOR_PARTITION_COUNT/COUNT").toInt();
    qDebug("nor part count: %d\n", m_norPartCnt);
    
    memset(m_norPartInfo_ext, 0, sizeof(m_norPartInfo_ext));

    if(m_norPartCnt == 0)  // norflash no partitions
    {
        QString strTemp, strLog;
        ifstream partFile;
        int  partFileLen = 0;
        char temp[MAX_STRING_LEN];

        strLog = QString("parseNorCfg, path: %1").arg(m_norPath);
        upgLog(LOG_DEBUG, strLog);
        if(m_norPath.isEmpty())
        {
            strLog = QString("You did not select any nor image");
            upgLog(LOG_DEBUG, strLog);
            return -1;
        }

        strTemp = m_norPath;
        qstringToChar(strTemp, temp);
        partFile.open(temp);
        if (!partFile.is_open())
        {
            strLog = QString("parseNorCfg, open file fail: %1").arg(strTemp);
            upgLog(LOG_ERROR, strLog);
            return -1;
        }
        partFile.seekg(0, ios::end);
        partFileLen = partFile.tellg();
        partFile.seekg(0, ios::beg);

        strcpy(m_norPartInfo_ext[0].partName,"sflash");
        strcpy(m_norPartInfo_ext[0].partFileName,temp);
        m_norPartInfo_ext[0].partFileSize = partFileLen;
        m_norPartInfo_ext[0].partFileAlignSize = align(ALIGN_64KB, partFileLen);
        m_norPartCnt = 1;
        goto LOG;
    }

    for(i=0; i<m_norPartCnt; i++)
    {
        //norPartName
        strIni.sprintf("NOR_PARTITION%d", i);
        strIni.append("/NAME");
        qstringToChar(settings.value(strIni).toString(), m_norPartInfo_ext[i].partName);
        //norPartSize
        strIni.sprintf("NOR_PARTITION%d", i);
        strIni.append("/SIZE");
        qstringHexToInt(settings.value(strIni).toString(), &m_norPartInfo_ext[i].partSize);
        //norPartFileName
        strIni.sprintf("NOR_PARTITION%d", i);
        strIni.append("/FILE");
        qstringToChar(settings.value(strIni).toString(), m_norPartInfo_ext[i].partFileName);
        
        //norPartFileSize
        strTemp = path;
        strTemp.replace(QString("ALI.ini"),QString(m_norPartInfo_ext[i].partFileName));
        qstringToChar(strTemp, temp);
        norPartFileName.open(temp);
        if (!norPartFileName.is_open())
        {
            strLog = QString("parseNorCfg, open file fail: %1").arg(strTemp);
            upgLog(LOG_ERROR, strLog);
            //return -1;
            m_norPartInfo_ext[i].partFileSize = 0;
            m_norPartInfo_ext[i].partFileAlignSize = 0;
        }
        else
        {
            norPartFileName.seekg(0, ios::end);
            partFileLen = norPartFileName.tellg();
            norPartFileName.seekg(0, ios::beg);
            m_norPartInfo_ext[i].partFileSize = partFileLen;
            //m_nandPartInfo[i].partFileAlignSize = align128K(partFileLen);
            m_norPartInfo_ext[i].partFileAlignSize = align(ALIGN_64KB, partFileLen);
        } 
       norPartFileName.close();
    }

LOG:
    for(i=0; i<m_norPartCnt; i++)
    {
        qDebug("[Part%d]\tPartName:%s\tpartSize:0x%x\tPartFile:%s\tPartFileSize:0x%x\tPartFileAlignSize:0x%x",
               i, m_norPartInfo_ext[i].partName, m_norPartInfo_ext[i].partSize,
               m_norPartInfo_ext[i].partFileName, m_norPartInfo_ext[i].partFileSize, m_norPartInfo_ext[i].partFileAlignSize);
        /*strLog = QString("[Part%1]\tPartName:%2\tpartSize:%3\tPartFile:%4\tPartFileSize:%5\tPartFileAlignSize:%6")\
                .arg(i).arg(m_norPartInfo_ext[i].partName).arg(m_norPartInfo_ext[i].partSize,0,16)\
                .arg(m_norPartInfo_ext[i].partFileName).arg(m_norPartInfo_ext[i].partFileSize,0,16).arg(m_norPartInfo_ext[i].partFileAlignSize,0,16);
        upgLog(LOG_DEBUG, strLog);*/
    }

    return 0;
}


int ALiUpgPackTool::parseNandCfg(QString path)
{
    int i = 0;
    QString strTemp, strIni, strLog;
    ifstream nandPartFileName;
    int  partFileLen = 0;
    char temp[MAX_STRING_LEN];
    QString loader;
    int hasLoader = 0;

    QFileInfo nandCfgFileInfo(path);
    if (!nandCfgFileInfo.exists())
    {
        strLog = QString("parseNandCfg, open file fail: %1").arg(path);
        upgLog(LOG_ERROR, strLog);
        return -1;
    }
    strLog = QString("parseNandCfg, config file: %1").arg(path);
    upgLog(LOG_DEBUG, strLog);

    QSettings settings(path, QSettings::IniFormat);

    m_nandPartCnt = settings.value("PARTITION-COUNT/COUNT").toInt();
    if(m_nandPartCnt != 0)
        m_nandPartCnt += 1; //PARTITION-COUNT/COUNT is not include uboot, so add 1 here
        
    loader = settings.value("STARTUP-FILE/LOADER").toString();
    if (loader.compare("false", Qt::CaseInsensitive) && !loader.isEmpty()) {
        hasLoader = 1;
        qDebug() << "ADD UBOOT";
    } else {
        hasLoader = 0;
    }

    memset(m_nandPartInfo, 0, sizeof(m_nandPartInfo));

    for(i=0; i<m_nandPartCnt; i++)
    {
        if(0 == i)
        {
            if (hasLoader) {
                strcpy(m_nandPartInfo[i].partName, "boot");
                qstringHexToInt(settings.value("ALI-PRIVATE-PARTITION0/SIZE").toString(), &m_nandPartInfo[i].partSize);
                qstringToChar(settings.value("STARTUP-FILE/LOADER").toString(), m_nandPartInfo[i].partFileName);

                nandPartFileName.open(m_nandPartInfo[i].partFileName);
                if (!nandPartFileName.is_open())
                {
                    strLog = QString("parseNandCfg, open file fail: %1").arg(m_nandPartInfo[i].partFileName);
                    upgLog(LOG_ERROR, strLog);
                    return -1;
                }
                nandPartFileName.seekg(0, ios::end);
                partFileLen = nandPartFileName.tellg();
                nandPartFileName.seekg(0, ios::beg);
                m_nandPartInfo[i].partFileSize = partFileLen;
                //m_nandPartInfo[i].partFileAlignSize = align128K(partFileLen);
                m_nandPartInfo[i].partFileAlignSize = align(ALIGN_128KB, partFileLen);
                nandPartFileName.close();
                qDebug() << "ADD UBOOT";
                continue;
            }
        }
        
        //PartName
        strIni.sprintf("PARTITION%d", i);
        strIni.append("/NAME");
        qstringToChar(settings.value(strIni).toString(), m_nandPartInfo[i].partName);
        //PartSize
        strIni.sprintf("PARTITION%d", i);
        strIni.append("/SIZE");
        qstringHexToInt(settings.value(strIni).toString(), &m_nandPartInfo[i].partSize);
        //nandPartFileName
        strIni.sprintf("PARTITION%d", i);
        strIni.append("/FILE");
        qstringToChar(settings.value(strIni).toString(), m_nandPartInfo[i].partFileName);
        
        //nandPartFileSize
        strTemp = path;
        strTemp.replace(QString("ALI.ini"),QString(m_nandPartInfo[i].partFileName));
        qstringToChar(strTemp, temp);
        nandPartFileName.open(temp);
        if (!nandPartFileName.is_open())
        {
            strLog = QString("parseNandCfg, open file fail: %1").arg(strTemp);
            upgLog(LOG_ERROR, strLog);
            m_nandPartInfo[i].partFileSize = 0;
            m_nandPartInfo[i].partFileAlignSize = 0;
        }
        else
        {
            nandPartFileName.seekg(0, ios::end);
            partFileLen = nandPartFileName.tellg();
            nandPartFileName.seekg(0, ios::beg);
            m_nandPartInfo[i].partFileSize = partFileLen;
            //m_nandPartInfo[i].partFileAlignSize = align128K(partFileLen);
            m_nandPartInfo[i].partFileAlignSize = align(ALIGN_128KB, partFileLen);
        }
        nandPartFileName.close();
    }

    for(i=0; i<m_nandPartCnt; i++)
    {
        qDebug("[Part%d]\tpartName:%s\tPartSize:0x%x\tPartFile:%s\tPartFileSize:0x%x\tPartFileAlignSize:0x%x",
               i, m_nandPartInfo[i].partName, m_nandPartInfo[i].partSize,
               m_nandPartInfo[i].partFileName, m_nandPartInfo[i].partFileSize, m_nandPartInfo[i].partFileAlignSize);
        /*strLog = QString("[Part%1]\tpartName:%2\tPartSize:%3\tPartFile:%4\tPartFileSize:%5\tPartFileAlignSize:%6")\
                .arg(i).arg(m_nandPartInfo[i].partName).arg(m_nandPartInfo[i].partSize,0,16)\
                .arg(m_nandPartInfo[i].partFileName).arg(m_nandPartInfo[i].partFileSize,0,16).arg(m_nandPartInfo[i].partFileAlignSize,0,16);
        upgLog(LOG_DEBUG, strLog);
        */
    }

    return 0;
}

int ALiUpgPackTool::check_partitions_conflict(void)
{
    int  i = 0;
    int  j = 0;
    bool contain_upgfw_group = false;
    bool contain_mainfw_group = false;
    int  ret = 0;

    for(i=0; i<m_nandPartCnt; i++)
    {
        if(m_nandPartSelect[i])
        {
            for(j=0; j<m_mainfw_parts_cnt; j++)
            {
                if(0 == strcmp(m_nandPartInfo[i].partName, m_mainfw_parts_group[j]))
                {
                    contain_mainfw_group = true;
                    break;
                }
            }

            for(j=0; j<m_upgfw_parts_cnt; j++)
            {
                if(0 == strcmp(m_nandPartInfo[i].partName, m_upgfw_parts_group[j]))
                {
                    contain_upgfw_group = true;
                    break;
                }
            }

            if(contain_upgfw_group && contain_mainfw_group)
            {
                ret = -1;
                break;
            }
        }

    }

    return ret;
}

void ALiUpgPackTool::start_pack()
{
    if(m_isPacking)
    {
        return;
    }
    m_isPacking = true;

    threadGenerateUpgPkt_cli(this);
}

void ALiUpgPackTool::threadGenerateUpgPkt_cli(void *arg)
{
    int   i = 0;
    int   partIdx = 0;
    int   offset = 0;
    char  imgPath[MAX_STRING_LEN];
    char  otaImgPath[MAX_STRING_LEN];
    char  path[MAX_STRING_LEN];
    int   fileSize = 0;
    int   remainSize = 0;
    int   upgNandPartCnt = 0;
    int   upgNorPartCnt = 0;
    FILE *fpOutImage = NULL;
    FILE *fpOutImageTmp = NULL;
    FILE *fpIn = NULL;
    unsigned char *buf = NULL;
    QString filePath;
    QString strLog;
    alislupg_header_t header;
    unsigned int crc = 0;
    int crc_piece_size = 0;
    int ret = SUCCESS;
    int mainver = 0;
    int upgver = 0;
    int bootver = 0;
    size_t size;

    //ui->startButton->setEnabled(false);

    if(NULL == arg)
    {
        m_isPacking = false;
        return;
    }

    /* input valid check */
    ret = inputValidCheck_cli();
    if(SUCCESS != ret)
    {
        if(ERROR_PART_NOT_SELECTED == ret)
        {
            strLog = QString("please select partitions!");
        }
        else if(ERROR_PART_CONFLICT == ret)
        {
            strLog = QString("can not select mainfw and upgfw partitions together!");
            strLog += QString("\nmainfw partitions: ");
            for(i=0; i< m_mainfw_parts_cnt; i++)
            {
                strLog += QString("%1/").arg(m_mainfw_parts_group[i]);
            }
            strLog += QString("\nupgfw partitions: ");
            for(i=0; i< m_upgfw_parts_cnt; i++)
            {
                strLog += QString("%1/").arg(m_upgfw_parts_group[i]);
            }
        }
        else if(ERROR_VERSION_NOT_INPUT == ret)
        {
            strLog = QString("please input version!");
        }
        else
        {
            strLog = QString("please check the input data!");
        }
        upgLog(LOG_ERROR, strLog);
        m_isPacking = false;
        return;
    }

    strLog = QString("generating upgrade package, please wait...");
    upgLog(LOG_WARNING, strLog);

    /* Step1: generate the upgrade package */
    filePath = m_imageDir;
    filePath.append("/");
    filePath.append(m_imgName);
    qstringToChar(filePath, imgPath);
    fpOutImage = fopen(imgPath, "wb+");
    if (NULL == fpOutImage)
    {
        strLog = QString("create file fail: %1").arg(imgPath);
        upgLog(LOG_ERROR, strLog);
        m_isPacking = false;
        return;
    }

    /*
     * header
     */
    memset(&header, 0, sizeof(header));
    /* upg_ver */
    header.upg_ver = 40;
    /* package_ver */
    QString mainFwVer = m_mainFwVer;
    qstringDecToInt(mainFwVer, &mainver);
    QString upgFwVer = m_upgFwVer;
    qstringDecToInt(upgFwVer, &upgver);
    QString bootVer = m_bootVer;
    qstringDecToInt(bootVer, &bootver);
    header.package_ver = (bootver<<16 | upgver<<8 | mainver<<0);
    /* part_cnt */
    if(!m_imageDir.isEmpty())
    {
        for(partIdx=0; partIdx<m_nandPartCnt; partIdx++)
        {
            if(m_nandPartSelect[partIdx])
            {
                upgNandPartCnt++;
            }
        }
    }
    if(!m_nor_images.isEmpty())
    {
        for(partIdx=0; partIdx<m_norPartCnt; partIdx++)
        {
            if(m_norPartSelect[partIdx])
            {
                upgNorPartCnt++;
            }
        }
    }
    header.part_cnt = upgNandPartCnt + upgNorPartCnt;
    /* ota_down_buf_size */
    header.ota_down_buf_size = m_otaUserParam.down_buffer_size;
    /* image_info */
    offset = sizeof(header);
    if(!m_imageDir.isEmpty())
    {
        for(partIdx = 0; partIdx < m_nandPartCnt; partIdx++)
        {
            if(m_nandPartSelect[partIdx])
            {
                strcpy(header.image_info[i].image_name, m_nandPartInfo[partIdx].partName);
                header.image_info[i].image_offset = offset;
                if(m_nandPartInfo[partIdx].partFileAlignSize != 0)
                {
                    header.image_info[i].image_size = m_nandPartInfo[partIdx].partFileAlignSize;
                    offset += m_nandPartInfo[partIdx].partFileAlignSize;
                }
                else
                {
                    header.image_info[i].image_size = m_nandPartInfo[partIdx].partSize;
                    offset += m_nandPartInfo[partIdx].partSize;
                }
                i++;
            }
        }
    }
    
    if(!m_nor_images.isEmpty())
    {
        //qDebug("threadGenerateUpgPkt_cli, nor header 1, nor part count: %d", m_norPartCnt);
        for(partIdx = 0; partIdx < m_norPartCnt; partIdx++)
        {
            //qDebug("partidx: %d, select: %d", partIdx, m_norPartSelect[partIdx]);
            if(m_norPartSelect[partIdx])
            {
                strcpy(header.image_info[i].image_name, m_norPartInfo_ext[partIdx].partName);
                header.image_info[i].image_offset = offset;
                if(m_norPartInfo_ext[partIdx].partFileAlignSize != 0)
                {
                    header.image_info[i].image_size = m_norPartInfo_ext[partIdx].partFileAlignSize;
                    offset += m_norPartInfo_ext[partIdx].partFileAlignSize;
                }
                else
                {
                    header.image_info[i].image_size = m_norPartInfo_ext[partIdx].partSize;
                    offset += m_norPartInfo_ext[partIdx].partSize;
                }
                i++;
            }
        }
    }
    fwrite(&header, sizeof(header), 1, fpOutImage);

    buf = (unsigned char *)malloc(BYTES_READ_PERTIME);

    /*
     * data
     */
    /* nand data */
    if(!m_imageDir.isEmpty())
    {
        for(partIdx=0; partIdx<m_nandPartCnt; partIdx++)
        {
            if(m_nandPartSelect[partIdx])
            {

                filePath = m_imageDir;
                filePath.append("/");
                filePath.append(m_nandPartInfo[partIdx].partFileName);
                qstringToChar(filePath, path);

                fpIn = NULL;
                if (m_nandPartInfo[partIdx].partFileSize > 0)
                {
                    fpIn = fopen(path, "rb");
                    if (NULL == fpIn)
                    {
                        strLog = QString("threadGenerateUpgPkt_cli 1, open file fail: %1").arg(filePath);
                        upgLog(LOG_ERROR, strLog);
                        free(buf);
                        buf = NULL;
                        fclose(fpOutImage);
                        //ui->startButton->setEnabled(true);
                        m_isPacking = false;
                        return;
                    }
                }
                fileSize = m_nandPartInfo[partIdx].partFileSize;
                remainSize = fileSize;

                if(NULL != fpIn)
                    fseek(fpIn, 0, SEEK_SET);
                if(fileSize == 0)
                    remainSize = m_nandPartInfo[partIdx].partSize;

                /*qDebug("partidx: %d, filesize: 0x%08x, remainsize: 0x%08x, alignsize: 0x%08x",\
                    partIdx, m_nandPartInfo[partIdx].partFileSize, remainSize, m_nandPartInfo[partIdx].partFileAlignSize);
                */
                
                while(remainSize>0)
                {
                    if(remainSize >= BYTES_READ_PERTIME)
                    {
                        
                        if(fileSize != 0)
                        {
                            memset(buf, 0x00, BYTES_READ_PERTIME);
                            size = fread(buf, BYTES_READ_PERTIME, 1, fpIn);
                        }
                        else
                        {
                            memset(buf, 0xFF, BYTES_READ_PERTIME);
                        }

                        size = fwrite(buf, BYTES_READ_PERTIME, 1, fpOutImage);
                        remainSize -= BYTES_READ_PERTIME;
                        if (size != 1)
                        {
                            qDebug() << "Write File error!!!";
                        }
                    }
                    else
                    {
                        if(fileSize != 0)
                        {
                            memset(buf, 0x00, remainSize);
                            size = fread(buf, remainSize, 1, fpIn); 
                        }
                        else
                        {
                            memset(buf, 0xFF, remainSize);
                        }

                        size = fwrite(buf, remainSize, 1, fpOutImage);
                        remainSize = 0;
                    }
                }

                if(m_nandPartInfo[partIdx].partFileAlignSize > fileSize)
                {
                    memset(buf, 0xff, m_nandPartInfo[partIdx].partFileAlignSize-fileSize);
                    size = fwrite(buf, m_nandPartInfo[partIdx].partFileAlignSize-fileSize, 1, fpOutImage);
                    //qDebug("partidx: %d, remainalignsize", partIdx);
                }

                if(NULL != fpIn)
                    fclose(fpIn);
            }
        }
    }
    /* nor data */
    if(!m_nor_images.isEmpty())
    {
        for(partIdx=0; partIdx<m_norPartCnt; partIdx++)
        {
            if(m_norPartSelect[partIdx])
            {
                filePath = m_imageDir;
                filePath.append("/");
                filePath.append(m_norPartInfo_ext[partIdx].partFileName);
                qstringToChar(filePath, path);
                fpIn = NULL;
                if(m_norPartInfo_ext[partIdx].partFileSize > 0)
                {
                    fpIn = fopen(path, "rb");
                    if (NULL == fpIn)
                    {
                        strLog = QString("threadGenerateUpgPkt_cli 1, open file fail: %1").arg(filePath);
                        upgLog(LOG_ERROR, strLog);
                        free(buf);
                        buf = NULL;
                        fclose(fpOutImage);
                        //ui->startButton->setEnabled(true);
                        m_isPacking = false;
                        return;
                    }
                }
                fileSize = m_norPartInfo_ext[partIdx].partFileSize;
                remainSize = fileSize;
                if(NULL != fpIn)
                    fseek(fpIn, 0, SEEK_SET);

                if(fileSize == 0)
                    remainSize = m_norPartInfo_ext[partIdx].partSize;

                //qDebug("idx: %d, filesize: %d, remainsize: %d\n", partIdx, fileSize, remainSize);

                while(remainSize>0)
                {
                    if(remainSize >= BYTES_READ_PERTIME)
                    {
                        if(fileSize != 0)
                        {
                            memset(buf, 0x00, BYTES_READ_PERTIME);
                            size = fread(buf, BYTES_READ_PERTIME, 1, fpIn);
                        }
                        else
                        {
                            memset(buf, 0xFF, BYTES_READ_PERTIME);
                        }

                        size = fwrite(buf, BYTES_READ_PERTIME, 1, fpOutImage);
                        remainSize -= BYTES_READ_PERTIME;
                        if (size != 1)
                        {
                            qDebug() << "Write File error!!!";
                        }
                    }
                    else
                    {
                        if(fileSize != 0)
                        {
                            memset(buf, 0x00, remainSize);
                            size = fread(buf, remainSize, 1, fpIn);
                        }
                        else
                        {
                            memset(buf, 0xFF, remainSize);
                        }

                        size = fwrite(buf, remainSize, 1, fpOutImage);
                        remainSize = 0;
                    }
                }
                if(m_norPartInfo_ext[partIdx].partFileAlignSize > fileSize)
                {
                    memset(buf, 0xff, m_norPartInfo_ext[partIdx].partFileAlignSize-fileSize);
                    size = fwrite(buf, m_norPartInfo_ext[partIdx].partFileAlignSize-fileSize, 1, fpOutImage);
                }

                if(NULL != fpIn)
                    fclose(fpIn);
            }
        }
    }

    free(buf);
    buf = NULL;
    fclose(fpOutImage);

    strLog = QString("upgrade package: %1").arg(imgPath);
    upgLog(LOG_DEBUG, strLog);

    /* Step2: generate ota package from the upgrade package with CRC*/
    if(m_genOtaPktSelect)
    {
        /* insert CRC to the upgrade package */
        filePath = m_imageDir;
        filePath.append("/");
        filePath.append(m_imgName);
        qstringToChar(filePath, imgPath);
        fpOutImage = fopen(imgPath, "rb");
        if (NULL == fpOutImage)
        {
            strLog = QString("create file fail: %1").arg(imgPath);
            upgLog(LOG_ERROR, strLog);
            //ui->startButton->setEnabled(true);
            m_isPacking = false;
            return;
        }
        fseek(fpOutImage, 0, SEEK_END);
        fileSize = ftell(fpOutImage);

        filePath = m_imageDir;
        filePath.append("/");
        filePath.append(m_imgName);
        filePath.append(".temp");
        qstringToChar(filePath, imgPath);
        fpOutImageTmp = fopen(imgPath, "wb+");
        if (NULL == fpOutImageTmp)
        {
            strLog = QString("create file fail: %1").arg(imgPath);
            upgLog(LOG_ERROR, strLog);
            //ui->startButton->setEnabled(true);
            m_isPacking = false;
            return;
        }

        MG_Setup_CRC_Table();
        remainSize = fileSize;
        fseek(fpOutImage, 0, SEEK_SET);
        crc_piece_size = (m_otaUserParam.down_buffer_size*0x100000/OTA_TS_MODULE_ZISE)*OTA_TS_MODULE_VALIE_ZISE - 4;
        buf = (unsigned char *)malloc(crc_piece_size);
        while(remainSize>0)
        {
            memset(buf, 0x00, crc_piece_size);
            if(remainSize >= crc_piece_size)
            {
                size = fread(buf, crc_piece_size, 1, fpOutImage);
                size = fwrite(buf, crc_piece_size, 1, fpOutImageTmp);
                crc = MG_Table_Driven_CRC(0xFFFFFFFF, buf, crc_piece_size);
                size = fwrite(&crc, 4, 1, fpOutImageTmp);
                remainSize -= crc_piece_size;
            }
            else
            {
                size = fread(buf, remainSize, 1, fpOutImage);
                size = fwrite(buf, remainSize, 1, fpOutImageTmp);//align to one piece
                crc = MG_Table_Driven_CRC(0xFFFFFFFF, buf, remainSize);
                size = fwrite(&crc, 4, 1, fpOutImageTmp);
                remainSize = 0;
            }

        }
        free(buf);
        buf = NULL;
        fclose(fpOutImage);
        fclose(fpOutImageTmp);


        /* generate ota package form temp image */
        filePath = m_imageDir;
        filePath.append("/");
        filePath.append(m_imgName);
        filePath.append(".temp");
        qstringToChar(filePath, imgPath);
        filePath = m_imageDir;
        filePath.append("/");
        filePath.append(m_otaImgName);
        qstringToChar(filePath, otaImgPath);
        if(0 != gen_otapkt_start(&m_otaUserParam, imgPath, otaImgPath))
        {
            strLog = QString("generate ota package fail!");
            upgLog(LOG_ERROR, strLog);
        }
        strLog = QString("ota upgrade package output:\n%1").arg(otaImgPath);
        upgLog(LOG_DEBUG, strLog);
    }

    strLog = QString("########## All done! ##########");
    upgLog(LOG_DEBUG, strLog);

    m_isPacking = false;
    //ui->startButton->setEnabled(true);
    return;

}

void ALiUpgPackTool::upgLog(int logType, QString str)
{
    if(LOG_ERROR == logType)
    {
        qDebug() << "Error:" << str <<endl;
    }
    else if(LOG_WARNING == logType)
    {
        qDebug() << "Warning:" << str <<endl;
    }
    else
    {
        qDebug() << str <<endl;
    }
}

int ALiUpgPackTool::load_nandFlash(QString directory)
{
    QString strLog;
    int i;

    if (!directory.isEmpty())
    {
        QString nandCfgFileName(directory);
        nandCfgFileName.append("/ALI.ini");
        QFileInfo nandCfgFileInfo(nandCfgFileName);
        if (!nandCfgFileInfo.exists())
        {
            strLog = QString("load_nandFlash, open file fail: %1").arg(nandCfgFileName);
            upgLog(LOG_ERROR, strLog);
            return -1;
        }
        if(0 == parseNandCfg(nandCfgFileName))
        {
            updatePartDisp();
//            ui->startButton->setEnabled(true);
            for(i=0; i<m_nandPartCnt; i++)
            {
                if ((m_nandPartSelect[i] == true) && !m_nand_images.contains(m_nandPartInfo[i].partName, Qt::CaseInsensitive))
                {
                    m_nandPartSelect[i] = false;
                }
                else
                {
                    m_totalPartSize += m_nandPartInfo[i].partSize;

                    if(m_nandPartInfo[i].partFileAlignSize != 0)
                        m_totalFileSize += m_nandPartInfo[i].partFileAlignSize;
                    else
                        m_totalFileSize += m_nandPartInfo[i].partSize;
                    //qDebug("load_norFlash, select all part size: 0x%08x, file size: 0x%08x, ", m_totalPartSize, m_totalFileSize);
                }

                qDebug("load_nandFlash, i: %d,\t select: %d,  part: %s", i, m_nandPartSelect[i], m_nandPartInfo[i].partName);
            }
        }
    }
    else
    {
        strLog = QString("You did not select any directory");
        upgLog(LOG_ERROR, strLog);
    }

    qDebug("load_nandFlash, select nand part size: 0x%08x, file size: 0x%08x, ", m_totalPartSize, m_totalFileSize);

    return 0;
}

int ALiUpgPackTool::load_norFlash(QString directory)
{
    QString strLog;
    int i;

    if (!directory.isEmpty())
    {
        QString norCfgFileName(directory);
        norCfgFileName.append("/ALI.ini");
        QFileInfo norCfgFileInfo(norCfgFileName);
        if (!norCfgFileInfo.exists())
        {
            strLog = QString("load_norFlash, open file fail: %1").arg(norCfgFileName);
            upgLog(LOG_ERROR, strLog);
            return -1;
        }
        if(0 == parseNorCfg(norCfgFileName))
        {
            updatePartDisp_nor();
//            ui->startButton->setEnabled(true);
            for(i=0; i<m_norPartCnt; i++)
            {
                /*qDebug() << "m_nor_images: " << m_nor_images \
                 "\nnor part: "<< i << ", norPartName: " << m_norPartInfo_ext[i].partName \
                 ", part file name: " << m_norPartInfo_ext[i].partFileName << ", select: " << m_norPartSelect[i] <<endl;*/
 
                if ((m_norPartSelect[i] == true) \
                    && !m_nor_images.contains(m_norPartInfo_ext[i].partName, Qt::CaseInsensitive) \
                    && !m_nor_images.contains(m_norPartInfo_ext[i].partFileName, Qt::CaseInsensitive))
                {
                    m_norPartSelect[i] = false;
                }
                else
                {
                    m_totalPartSize += m_norPartInfo_ext[i].partSize;
                    if(m_norPartInfo_ext[i].partFileAlignSize != 0)
                        m_totalFileSize += m_norPartInfo_ext[i].partFileAlignSize;
                    else
                        m_totalFileSize += m_norPartInfo_ext[i].partSize;
                    //qDebug("load_norFlash, select all part size: 0x%08x, file size: 0x%08x, ", m_totalPartSize, m_totalFileSize);
                }
            }

            for(i=0; i<m_norPartCnt; i++)
            {
                qDebug("load_norFlash, i: %d,\t select: %d, part: %s", i, m_norPartSelect[i], m_norPartInfo_ext[i].partName);
            }
        }
    }
    else
    {
        strLog = QString("You did not select any directory");
        upgLog(LOG_ERROR, strLog);
    }

    qDebug("load_norFlash, select all part size: 0x%08x, file size: 0x%08x, ", m_totalPartSize, m_totalFileSize);

    return 0;
}

int ALiUpgPackTool::inputValidCheck_cli(void)
{
    int i = 0, j = 0;
    QString mainFwVer, upgFwVer, bootVer;
    QString norPath;

    if(m_nor_images.isEmpty() && m_imageDir.isEmpty())
    {
        return ERROR_PART_NOT_SELECTED;
    }

    if(!m_imageDir.isEmpty())
    {
        for(i=0; i<MAX_NAND_PART; i++)
        {
            if(m_nandPartSelect[i])
            {
            	//qDebug("inputValidCheck_cli, i: %d, break", i);
                break;
            }
        }

        for(j=0; j<MAX_NOR_PART; j++)
        {
            if(m_norPartSelect[j])
            {
            	//qDebug("inputValidCheck_cli, j: %d, break", j);
                break;
            }
        }
    }

    //qDebug("inputValidCheck_cli, i: %d, max nand part: %d\n", i, MAX_NAND_PART);
    if(MAX_NAND_PART == i && MAX_NOR_PART == j)
    {
        return ERROR_PART_NOT_SELECTED;
    }

    if((m_mainFwVer == "0") && (m_upgFwVer == "0") && (m_bootVer == "0"))
    {
        return ERROR_VERSION_NOT_INPUT;
    }

    if(!m_imageDir.isEmpty())
    {
        if(0 != check_partitions_conflict())
        {
            return ERROR_PART_CONFLICT;
        }
    }

    return SUCCESS;
}
