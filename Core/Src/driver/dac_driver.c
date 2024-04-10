/**
  ******************************************************************************
  * @file    dac_driver.c
  * @author  Bianchi Davide
  * @brief   This file contains all the low level functions that are used to set
  * 		 the DAC CS43L22 on the stm32 board
  ******************************************************************************
**/

#include "driver/dac_driver.h"

/*
 * Initialisation
 */

uint8_t CS43L22_Init(CS43L22 *dev, I2C_HandleTypeDef *i2cHandle) {
	/* Recommended Power-Up sequence (4.9 - p.32) */
	/* 2. Bring RESET high */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);

	dev->i2cHandle = i2cHandle;
	HAL_StatusTypeDef status;

	uint8_t regData;
	status = CS43L22_ReadRegister(dev, DEVICE_ID_ADDR, &regData);

	/* Checking the device ID (7.1.1 - p.38)*/
	if ((regData >> 3) != DEVICE_ID) {
		return 255;
	}

	/* Checking the device revision (7.1.2 - p.38)*/
	if ((regData & 0x07) != REV_ID_0 && (regData & 0x07) != REV_ID_1 && (regData & 0x07) != REV_ID_2 && (regData & 0x07) != REV_ID_3) {
		return 255;
	}

	/* Initialization and configuration of the Register Bank */
	// 3. Write 0x01 to register 0x02 (Power Ctl. 1)
	regData = 0x01;
	status = CS43L22_WriteRegister(dev, POWER_CTL_1, &regData);

	/* Configuring of the desired register settings */
	// Cap 7.3. Power Control 2
	regData = 0xAF;	// 10101111 -> headphone channel on and speaker channel off
	status = CS43L22_WriteRegister(dev, POWER_CTL_2, &regData);
	// Cap 7.5. Interface Control 1
	CS43L22_ReadRegister(dev, INTERFACE_CTL_1, &regData);
	regData &= 0xF0;
	regData |= 0x07; // 00000111 -> I2S up to 24 bit data, audio word = 16 bit
	status = CS43L22_WriteRegister(dev, INTERFACE_CTL_1, &regData);
	// Cap 7.10. Playback Control 1
	regData = 0xA0; // 10100000 -> headphone gain = 0.8399
	status = CS43L22_WriteRegister(dev, PLAYBACK_CTL_1, &regData);


	/* 4. Required initialization settings (4.11 - p.33) */
	// Write 0x99 to register 0x00
	regData = 0x99;
	status = CS43L22_WriteRegister(dev, 0x00, &regData);
	// Write 0x80 to register 0x47
	regData = 0x80;
	status = CS43L22_WriteRegister(dev, 0x47, &regData);
	// Write 1b to bit 7 in register 0x32
	status = CS43L22_ReadRegister(dev, 0x32, &regData);
	regData |= 0x80;
	status = CS43L22_WriteRegister(dev, 0x32, &regData);
	// Write 0b to bit 7 in register 0x32
	status = CS43L22_ReadRegister(dev, 0x32, &regData);
	regData &= 0x7F;
	status = CS43L22_WriteRegister(dev, 0x32, &regData);
	// Write 0x00 to register 0x00
	regData = 0;
	status = CS43L22_WriteRegister(dev, 0x00, &regData);

	/* 5. Apply MCLK at the appropriate frequency. By default it auto-detects external clock. */

	/* 6. Write 0x9E to register 0x02 (Power Ctl. 1) */
	regData = 0x9E;
	status = CS43L22_WriteRegister(dev, POWER_CTL_1, &regData);

}


/*
 * LOW-LEVEL FUNCTIONS
 */

HAL_StatusTypeDef CS43L22_ReadRegister(CS43L22 *dev, uint8_t reg, uint8_t *data) {
	return HAL_I2C_Mem_Read(dev->i2cHandle, I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}
HAL_StatusTypeDef CS43L22_ReadRegisters(CS43L22 *dev, uint8_t reg, uint8_t *data, uint8_t length) {
	return HAL_I2C_Mem_Read(dev->i2cHandle, I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}
HAL_StatusTypeDef CS43L22_WriteRegister(CS43L22 *dev, uint8_t reg, uint8_t *data) {
	return HAL_I2C_Mem_Write(dev->i2cHandle, I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

