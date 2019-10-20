/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsysinfo.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 14:51:42
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltflog.h>
#include <alislsysinfo.h>

/* kernel headers */
#include <ali_mtd_common.h>

/* Internal header */
#include "internal.h"

#if 0// ENABLE_DEBUG
static char *sysinfo_err[] = {
	"No error happen",
	"Invalid area for system information",
	"Prepare buffer firstly",
	"Can't seek to required area",
	"Can't open device",
};
#endif

/*
 *  Function Name:  alislsysinfo_construct
 *  @brief          
 *
 *  @param          desc   descriptor of system information
 *  @param          param  parameters for initialization
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode alislsysinfo_construct(alislsysinfo_desc_t *desc, alislsysinfo_param_t *param)
{
	void *dev = NULL;
	alislsysinfo_err_t err = SYSINFO_ERR_NONE;
	//off_t orig_offset = 0;
	alisl_retcode sto_err = STO_ERR_NONE;
	size_t actual_size = 0;
	alislsto_rw_param_t para;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	desc->idx = param->idx;
	desc->buf = param->buf;

	/* alislsto_construct(&dev); */
	if (STO_ERR_NONE != alislsto_open(&dev, STO_TYPE_NAND, O_RDWR))
	{
		err = SYSINFO_ERR_CANTOPENDEV;
		goto deinit;
	}

	//alislsto_get_offset(dev, &orig_offset);
	//if (STO_ERR_NONE != alislsto_lseek_ext(dev, STO_PRIVATE, 0))
	//{
	//	alislsto_set_offset(dev, orig_offset, true);
	//	err = SYSINFO_ERR_CANTSEEKSYSINFO;
	//	goto close;
	//}
	//alislsto_set_offset(dev, orig_offset, true);


	para.handle = dev;
	para.size = 0;
	para.offset = 0;
	para.whenence = -1;
	para.idx = STO_PRIVATE;
	para.flag = true;

#if ENABLE_DEBUG
	SL_DBG("handle=%#x\n", para.handle);
	SL_DBG("buf=%#x\n", &(desc->buf));
	SL_DBG("size=%#x\n", para.size);
	SL_DBG("offset=%#llx\n", para.offset);
	SL_DBG("whenence=%d\n", para.whenence);
	SL_DBG("idx=%d\n", para.idx);
	SL_DBG("read=%#x\n", actual_size);
	SL_DBG("flag=%d\n", para.flag);
#endif

	sto_err = alislsto_lock_read(para, desc->buf,  &actual_size);
	if (sto_err != STO_ERR_NONE) {
		goto close;
	}

	desc->priv = dev;
	sysinfo_parse(desc);
	goto out;

close:
	alislsto_close(dev, false);
deinit:
	/* alislsto_destruct(&dev);  */
out:
	return err;
}


/*
 *  Function Name:  alislsysinfo_destruct
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode alislsysinfo_destruct(alislsysinfo_desc_t *desc)
{
	void *dev = desc->priv;

	alislsto_close(dev, false);
	/* alislsto_destruct(&dev); */
	dev = NULL;
	desc->priv = NULL;
	desc->buf = NULL;

	return SYSINFO_ERR_NONE;
}


/*
 *  Function Name:  alislsysinfo_read
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static size_t alislsysinfo_read(alislsysinfo_desc_t *desc)
{
	sysinfo_map_t  *map = NULL;
	int            idx = desc->idx;
	int            idxbase = 0;
	void           *dev = desc->priv;
	size_t         read = 0;
	//off_t       orig_offset = 0;
	alisl_retcode err = STO_ERR_NONE;
	alislsto_rw_param_t para;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	if (NULL == desc->buf)
	{
		return -EINVAL;
	}

	map = sysinfo_getmap(desc);
	idxbase = sysinfo_getidxbase(desc);
	idx = desc->idx - idxbase;
	desc->size = 0;

	if (0 == map[idx].size) 
	{
		SL_DBG("In %s Size is zero\n", __FUNCTION__);
		return 0;
	}

	/*
	* To protect offset of upgrade writter, original offset should
	* be saved and restored after read system info.
	*/
	//alislsto_get_offset(dev, &orig_offset);
	//if (STO_ERR_NONE != alislsto_lseek_ext(dev, STO_PRIVATE, map[idx].offset))
	//{
	//	alislsto_set_offset(dev, orig_offset, true);
	//	return -EINVAL;
	//}
    //
	//desc->size = map[idx].size;
	//read = alislsto_read(dev, desc->buf, desc->size);
	//alislsto_set_offset(dev, orig_offset, true);

	para.handle = dev;
	para.size = map[idx].size;
	para.offset = map[idx].offset;
	para.whenence = -1;
	para.idx = STO_PRIVATE;
	para.flag = true;
#if ENABLE_DEBUG
	SL_DBG("[%s %d]\n", __FUNCTION__, __LINE__);
	SL_DBG("handle=%#x\n", para.handle);
	SL_DBG("buf=%#x\n", &(desc->buf));
	SL_DBG("size=%#x\n", para.size);
	SL_DBG("offset=%#llx\n", para.offset);
	SL_DBG("whenence=%d\n", para.whenence);
	SL_DBG("idx=%d\n", para.idx);
	SL_DBG("read=%#x\n", read);
	SL_DBG("flag=%d\n", para.flag);
#endif
	err = alislsto_lock_read(para, desc->buf, &read);
	if (err != STO_ERR_NONE || map[idx].size != read) {
		return -EINVAL;
	} else {
		desc->size = read;
	}
	
//SL_DBG will do nothing if disable DEBUG in Mini build,thus the warning that 'i  set but not used' occur
#if ENABLE_DEBUG
	int i = 0;
	SL_DBG("In %s Read\n", __FUNCTION__);
	SL_DBG("------------------------------\n");
	SL_DBG("Name...............%s\n", map[idx].name);
	SL_DBG("Offset.............0x%x\n", map[idx].offset);
	SL_DBG("Size...............0x%x\n", map[idx].size);
	SL_DBG("Data Read Len......0x%x\n", read);
	SL_DBG("------------------------------\n");
	for (i = 0; i < read; i++) 
	{
	    SL_DBG("%2x ", desc->buf[i]);
		if (15 == i % 16)
		{
		    SL_DBG("\n");
		}
	}
	SL_DBG("\n");
	SL_DBG("------------------------------\n");
#endif

	return read;
}


/*
 *  Function Name:  alislsysinfo_write
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static size_t alislsysinfo_write(alislsysinfo_desc_t *desc)
{
	sysinfo_map_t    *map = NULL;
	int              idx = desc->idx;
	int              idxbase = 0;
	void         *dev = desc->priv;
//	off_t        orig_offset = 0;
	size_t write = 0;
	alisl_retcode err = STO_ERR_NONE;
	alislsto_rw_param_t para;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	if (NULL == desc->buf)
	{
		return -EINVAL;
	}
	
	map = sysinfo_getmap(desc);
	idxbase = sysinfo_getidxbase(desc);
	idx = desc->idx - idxbase;

	if (0 == map[idx].size) 
	{
		SL_DBG("In %s Size is zero\n", __FUNCTION__);
		return 0;
	}

	/*
	* To protect offset of upgrade writter, original offset should
	* be saved and restored after write system info.
	*/
	//alislsto_get_offset(dev, &orig_offset);
	//if (STO_ERR_NONE != alislsto_lseek_ext(dev, STO_PRIVATE, map[idx].offset))
	//{
	//	alislsto_set_offset(dev, orig_offset, true);
	//	return -EINVAL;
	//}
    //
	//desc->size = map[idx].size;
    //
	//alislsto_write(dev, desc->buf, desc->size);
	//alislsto_set_offset(dev, orig_offset, true);

	para.handle = dev;
	para.size = map[idx].size;
	para.offset = map[idx].offset;
	para.whenence = -1;
	para.idx = STO_PRIVATE;
	para.flag = true;
#if ENABLE_DEBUG
	SL_DBG("[%s %d]\n", __FUNCTION__, __LINE__);
	SL_DBG("handle=%#x\n", para.handle);
	SL_DBG("buf=%#x\n", &(desc->buf));
	SL_DBG("size=%#x\n", para.size);
	SL_DBG("offset=%#llx\n", para.offset);
	SL_DBG("whenence=%d\n", para.whenence);
	SL_DBG("idx=%d\n", para.idx);
	SL_DBG("write=%#x\n", write);
	SL_DBG("flag=%d\n", para.flag);
#endif
	err = alislsto_lock_write(para, desc->buf, &write);
	if (err != STO_ERR_NONE || map[idx].size != write) {
		return -EINVAL;
	} else {
		desc->size = write;
	}
	
//SL_DBG will do nothing if disable DEBUG in Mini build,thus the warning that 'i  set but not used' occur
#if ENABLE_DEBUG
	int i = 0;
	SL_DBG("In %s Write\n", __FUNCTION__);
	SL_DBG("------------------------------\n");
	SL_DBG("Name...............%s\n", map[idx].name);
	SL_DBG("Offset.............0x%x\n", map[idx].offset);
	SL_DBG("Size...............0x%x\n", map[idx].size);
	SL_DBG("Data Write Len.....0x%x\n", desc->size);
	SL_DBG("------------------------------\n");
	for (i = 0; i < desc->size; i++)
	{
	    SL_DBG("%2x ", desc->buf[i]);
		if (15 == i % 16) 
		{
		    SL_DBG("\n");
		}
	}
	SL_DBG("\n");
	SL_DBG("------------------------------\n");
#endif

	return 0;
}

/**
 *  Function Name:  alislsysinfo_mac_read
 *  @brief          
 *
 *  @param          buf  mac buffer
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_mac_read(unsigned char *buf)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;
        
	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_MAC;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	alislsysinfo_read(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

/**
 *  Function Name:  alislsysinfo_mac_write
 *  @brief          
 *
 *  @param          buf  mac buffer
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_mac_write(unsigned char *buf)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;

	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_MAC;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	alislsysinfo_write(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

/**
 *  Function Name:  alislsysinfo_upginfo_read
 *  @brief          
 *
 *  @param          buf   upginfo buffer
 *  @param          size  read size
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_upginfo_read(unsigned char *buf, size_t size)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;

	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_UPGINFO;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	sysinfo_setsize(&desc, size);
	alislsysinfo_read(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

/**
 *  Function Name:  alislsysinfo_upginfo_read
 *  @brief          
 *
 *  @param          buf   upginfo buffer
 *  @param          size  write size
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_upginfo_write(unsigned char *buf, size_t size)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;

	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_UPGINFO;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	sysinfo_setsize(&desc, size);
	alislsysinfo_write(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

/**
 *  Function Name:  alislsysinfo_extinfo_read
 *  @brief          
 *
 *  @param          buf   extinfo buffer
 *  @param          size  read size
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_extinfo_read(unsigned char *buf, size_t size)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;

	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_EXTINFO;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	sysinfo_setsize(&desc, size);
	alislsysinfo_read(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

/**
 *  Function Name:  alislsysinfo_extinfo_read
 *  @brief          
 *
 *  @param          buf   extinfo buffer
 *  @param          size  write size
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_extinfo_write(unsigned char *buf, size_t size)
{
	if (!buf)
		return SYSINFO_ERR_NULLBUF;

	alislsysinfo_desc_t desc;
	alislsysinfo_param_t param;

	memset(&desc, 0, sizeof(alislsysinfo_desc_t));
	memset(&param, 0, sizeof(alislsysinfo_param_t));
	param.idx = SYSINFO_EXTINFO;
	param.buf = buf;

	alislsysinfo_construct(&desc, &param);
	sysinfo_setsize(&desc, size);
	alislsysinfo_write(&desc);
	alislsysinfo_destruct(&desc);

	return desc.size;
}

