/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#if defined(FSL_FEATURE_SOC_PORT_COUNT) && (FSL_FEATURE_SOC_PORT_COUNT)
#include "fsl_port.h"
#endif
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_smartdma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Whether the SW button is pressed */
volatile bool g_ButtonPress = false;
smartdma_keyscan_4x4_param_t smartdmaParam;                  /*!< SMARTDMA function parameters. */
volatile uint8_t g_samrtdma_stack[32];
volatile uint32_t g_keyscan_gpio_register[8] = {
		0x5009C000 + 0x60 + 2,/*ROW1, P3_2,Pin Data security Register*/
		0x5009C000 + 0x60 + 3,/*ROW2, P3_3,Pin Data security Register*/
		0x50041000 + 0x60 + 6,/*ROW3, P5_6,Pin Data security Register*/
		0x50096000 + 0x60 + 20,/*ROW4, P0_20,Pin Data security Register*/
		0x50096000 + 0x60 + 17, /*COL1, P0_17,Pin Data security Register */
		0x50096000 + 0x60 + 18,/*COL2, P0_18,Pin Data security Register*/
		0x50096000 + 0x60 + 16,/*COL3, P0_16,Pin Data security Register*/
		0x50096000 + 0x60 + 19,/*COL4, P0_19,Pin Data security Register*/
};
volatile uint32_t KeyValue[8]={0,0,0,0,0};
volatile uint32_t g_keyscan_interval = 1000;
volatile uint32_t g_keyscan_complete_flag=0;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SmartDMA_keyscan_callback(void *param){
	g_keyscan_complete_flag = 1;
	SMARTDMA_AccessShareRAM(1) ;

	if(KeyValue[0]& 0x01){PRINTF("Button 1  is pressed \r\n");}
	if(KeyValue[0]& 0x02){PRINTF("Button 4  is pressed \r\n");}
	if(KeyValue[0]& 0x04){PRINTF("Button 7  is pressed \r\n");}
	if(KeyValue[0]& 0x08){PRINTF("Button 0  is pressed \r\n");}

	if(KeyValue[1]& 0x01){PRINTF("Button 2  is pressed \r\n");}
	if(KeyValue[1]& 0x02){PRINTF("Button 5  is pressed \r\n");}
	if(KeyValue[1]& 0x04){PRINTF("Button 8  is pressed \r\n");}
	if(KeyValue[1]& 0x08){PRINTF("Button F  is pressed \r\n");}

	if(KeyValue[2]& 0x01){PRINTF("Button 3  is pressed \r\n");}
	if(KeyValue[2]& 0x02){PRINTF("Button 6  is pressed \r\n");}
	if(KeyValue[2]& 0x04){PRINTF("Button 9  is pressed \r\n");}
	if(KeyValue[2]& 0x08){PRINTF("Button E  is pressed \r\n");}

	if(KeyValue[3]& 0x01){PRINTF("Button A  is pressed \r\n");}
	if(KeyValue[3]& 0x02){PRINTF("Button B  is pressed \r\n");}
	if(KeyValue[3]& 0x04){PRINTF("Button C  is pressed \r\n");}
	if(KeyValue[3]& 0x08){PRINTF("Button D  is pressed \r\n");}
	SMARTDMA_AccessShareRAM(0);
 }

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Print a note to terminal. */
    PRINTF("\r\n SmartDMA keyscan example\r\n");
    /*give smartdma role with secure user and non-privilege*/
 	AHBSC->MASTER_SEC_LEVEL = (AHBSC->MASTER_SEC_LEVEL) | (0x2<<4);
 	AHBSC->MASTER_SEC_ANTI_POL_REG = (AHBSC->MASTER_SEC_ANTI_POL_REG) & (~(0x2<<4));
    /*set GPIO pins user permission with secure user and non-privilege*/
 	GPIO0->PCNP = (1 << 16)|(1 << 17)|(1 << 18)|(1 << 19)|(1 << 20);
 	GPIO3->PCNP = (1 << 2)|(1 << 3);
 	GPIO5->PCNP = (1 << 6);

	SMARTDMA_InitWithoutFirmware();
	SMARTDMA_InstallFirmware(SMARTDMA_KEYSCAN_MEM_ADDR,s_smartdmaKeyscanFirmware,
								SMARTDMA_KEYSCAN_FIRMWARE_SIZE);
	SMARTDMA_InstallCallback(SmartDMA_keyscan_callback, NULL);
	NVIC_EnableIRQ(SMARTDMA_IRQn);
	NVIC_SetPriority(SMARTDMA_IRQn, 3);

	smartdmaParam.smartdma_stack 	 = (uint32_t*)g_samrtdma_stack;
	smartdmaParam.p_gpio_reg  		 = (uint32_t*)g_keyscan_gpio_register;
	smartdmaParam.p_keyvalue  		 = (uint32_t*)KeyValue;
	smartdmaParam.p_keycan_interval  = (uint32_t*)&g_keyscan_interval;
	SMARTDMA_Boot(kSMARTDMA_Keyscan_4x4, &smartdmaParam, 0x2);

    while (1)
    {

    }
}
