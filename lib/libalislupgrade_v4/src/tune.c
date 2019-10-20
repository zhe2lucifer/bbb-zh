/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               tune.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:20:26
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "internal.h"


sem_t writersem;
sem_t sourcesem;
pthread_mutex_t  tune_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 *  Function Name:  upgrade_tune_init
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_init(alislupg_desc_t *desc)
{
	upg_tune_t *tune = desc->tune;

	sem_init(&tune->writersem, 0, 1);
	sem_init(&tune->sourcesem, 0, 0);

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_tune_deinit
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_deinit(alislupg_desc_t *desc)
{
	upg_tune_t *tune = desc->tune;

	SL_DBG("Deinit tune\n");
	sem_destroy(&tune->sourcesem);
	sem_destroy(&tune->writersem);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_waitfinish
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_waitfinish(alislupg_desc_t *desc)
{

#if 0
	upg_tune_t *tune = desc->tune;
	if (tune->writting || tune->total)
		sem_wait(&desc->oversem);
#else
	sem_wait(&desc->oversem);
#endif

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_setsource
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_setsource(alislupg_desc_t *desc)
{
//	upg_tune_t *tune = desc->tune;
	//upg_object_t *object = desc->object;

	// DO NOTHING NOW

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_setdest
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_setdest(alislupg_desc_t *desc)
{
	//upg_tune_t *tune = desc->tune;

	// DO NOTHING NOW

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_unsetsource
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_unsetsource(alislupg_desc_t *desc)
{
	//upg_tune_t *tune = desc->tune;

	// DO NOTHING NOW

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_unsetdest
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_unsetdest(alislupg_desc_t *desc)
{
//	upg_tune_t *tune = desc->tune;

	// DO NOTHING NOW

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_start
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;

	upgrade_writter_start(desc);
	object->f_start(desc);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_tune_end
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode 
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_tune_end(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;

	object->f_stop(desc);
	upgrade_writter_stop(desc);

	return ALISLUPG_ERR_NONE;
}
