/**
  ******************************************************************************
  * @file    dac_driver.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the dac_driver.c
  ******************************************************************************
**/


#ifndef DAC_DRIVER_H
#define DAC_DRIVER_H

#include "stm32f4xx_hal.h"	/* Needed for I2C */


/*
 * DEFINES/REGISTERS (p.36)
 */

#define I2C_ADDR				(0x4A << 1)	/* Read = (1001010)1 (0x95); Write = (1001010)0 (0x94) */
#define DEVICE_ID 				0x1C
#define REV_ID_0				0x0
#define REV_ID_1				0x1
#define REV_ID_2				0x2
#define REV_ID_3				0x3

#define DEVICE_ID_ADDR			0x01
#define POWER_CTL_1				0x02
#define POWER_CTL_2				0x04
#define CLOCKING_CTL			0x05
#define INTERFACE_CTL_1			0x06
#define INTERFACE_CTL_2			0x07
#define PASSTHROUGH_A_SEL		0x08
#define PASSTHROUGH_B_SEL		0x09
#define ZC_SR					0x0A
#define PASSTHROUGH_GANG		0x0C
#define PLAYBACK_CTL_1			0x0D
#define MISC_CTL				0x0E
#define PLAYBACK_CTL_2			0x0F

#define PASSTHROUGH_A_VOL		0x14
#define PASSTHROUGH_B_VOL		0x15
#define PCMA_VOL				0x1A
#define PCMB_VOL				0x1B
#define BEEP_FREQ				0x1C
#define BEEP_VOL				0x1D
#define BEEP_TONE				0x1E
#define TONE_CTL				0x1F

#define MASTER_A_VOL			0x20
#define MASTER_B_VOL			0x21
#define HEADPHONE_A_VOL			0x22
#define HEADPHONE_B_VOL			0x23
#define SPEAKER_A_VOL			0x24
#define SPEAKER_B_VOL			0x25
#define CHNL_MIXER				0x26
#define LIMIT_CTL_1				0x27
#define LIMIT_CTL_2				0x28
#define LIMITER_ATTACK_RATE		0x29
#define OVRFLW_CLK_STATUS		0x2E
#define BATTERY_COMP			0x2F
#define BATTERY_LVL				0x30
#define SPEAKER_STATUS			0x31
#define CHARGE_PUMP_FREQ		0x34

/*
 * DAC STRUCT
 */

typedef struct {
	/* I2C Handle*/
	I2C_HandleTypeDef *i2cHandle;
} CS43L22;

/*
 * INITIALISATION
 */
uint8_t CS43L22_Init(CS43L22 *dev, I2C_HandleTypeDef *i2cHandle);

/*
 * LOW-LEVEL FUNCTIONS
 */
HAL_StatusTypeDef CS43L22_ReadRegister(CS43L22 *dev, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef CS43L22_ReadRegisters(CS43L22 *dev, uint8_t reg, uint8_t *data, uint8_t length);
HAL_StatusTypeDef CS43L22_WriteRegister(CS43L22 *dev, uint8_t reg, uint8_t *data);

#endif
