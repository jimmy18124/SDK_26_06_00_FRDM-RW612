/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NBU_DBG_H_
#define _NBU_DBG_H_

/*! *********************************************************************************
 * Include
 ********************************************************************************** */

#if defined(gLoggingActive_d) && (gLoggingActive_d == 1)
#include "dbg_logging.h"
#endif

/* Enable Debug Ios */
#define gDbgUseDbgIos 0

/* selective debug ios by configuration */
#define gDbgUseDbgIosLp 0
//#define gDbgUseDbgIosPlt                 1

/* Enable dtest feature */
#define gDbgUseDtest 1

/*! *********************************************************************************
 * Public definitions
 ********************************************************************************** */

#if gDbgUseDbgIos
#undef BOARD_DBGINITDBGIO
#define BOARD_DBGINITDBGIO() NbuDbg_IO_Init()
#endif

#if gDbgUseDbgIosLp
#undef BOARD_DBGLPIOSET
#define BOARD_DBGLPIOSET NbuDbg_IO_Set
#endif

#if gDbgUseDbgIosPlt
#undef BOARD_DBGPLTIOSET
#define BOARD_DBGPLTIOSET NbuDbg_IO_Set
#endif

#undef BOARD_DBGINITDTEST
#define BOARD_DBGINITDTEST() NbuDbg_DtestInit()

#undef BOARD_DBGINITRFACTIVE
#define BOARD_DBGINITRFACTIVE() NbuDbg_RfActiveInit()

#if defined(gLoggingActive_d) && (gLoggingActive_d == 1)
#undef BOARD_DBGLOGCOUNTERRUNNING
#define BOARD_DBGLOGCOUNTERRUNNING DbgLogLlCounterRunning
#endif

#undef BOARD_DBGCONFIGINITNBU
#define BOARD_DBGCONFIGINITNBU(__x) NbuDbg_ConfigInit(__x)

/*! *********************************************************************************
 * Public type definitions
 ********************************************************************************** */

/*! *********************************************************************************
 * Public memory declarations
 ********************************************************************************** */

/*! *********************************************************************************
 * Public functions
 ********************************************************************************** */
void NbuDbg_IO_Init(void);
void NbuDbg_IO_Set(unsigned long io, unsigned short level);
void NbuDbg_DtestInit(void);
void NbuDbg_RfActiveInit(void);
void NbuDbg_ConfigInit(bool_t first_time);

#endif /* _NBU_DBG_H_ */