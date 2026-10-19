/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
#define BOARD_APP_UART              USART0
#define BOARD_APP_UART_FRG_CLK      \
    (&(const clock_frg_clk_config_t){0, kCLOCK_FrgPllDiv, 255, 0}) /*!< Select FRG0 mux as frg_pll */
#define BOARD_APP_UART_CLK_ATTACH   kFRG_to_FLEXCOMM0
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
