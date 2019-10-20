/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.h
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
* 	2.  Dec.19.2003		 Justin Wu		 Ver 0.2    Add ESC CMD macros.
*	3.  Sep.23.2005		 Justin Wu		 Ver 0.3    Add pan information.
*****************************************************************************/

#ifndef __ALISLINPUT_DBG_H__
#define __ALISLINPUT_DBG_H__


#ifndef __ASSEMBLER__

//#include <hld/pan/pan_dev.h>
#include <alislinput_dev.h>
#include <ali_front_panel_common.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*!@struct alislinput_dbg
   @brief front panel debug level
*/

typedef enum alislinput_dbg_level
{
	PAN_DBG_LEVEL_DEFAULT = 0x00,
	PAN_DBG_LEVEL_HLD 	  = 0x01,
    PAN_DBG_LEVEL_KERNEL = 0x02,
}alislinput_dbg_level_e;


/*!@struct pan_dbg
   @brief front panel debug information
*/
struct alislinput_dbg
{
	unsigned char repeat_enable;
	alislinput_device_status_e stats;

	/* ir */
	long ir_handle;
	long input_handle_ir;
	unsigned long ir_last_key_high;
	unsigned long ir_last_key_low;
	unsigned long ir_format;
	unsigned long auto_change;					/* auto change channel */
	unsigned long interval;						/* auto change channel interval */
	alislinput_dbg_level_e  ir_level;
	struct ali_fp_key_map_cfg ir_key_map;		/* ir key map */

	/* panel */
	long panel_handle;
	long input_handle_panel;
	unsigned long panel_last_key_high;
	unsigned long panel_last_key_low;
	alislinput_dbg_level_e  panel_level;
	struct ali_fp_key_map_cfg panel_key_map;		/* panel key map */
};

#ifdef __cplusplus
}
#endif
#endif

/*!
 @}
 */
#endif /* __ALISLINPUT_DBG_H__ */
