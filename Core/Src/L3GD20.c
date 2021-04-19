/*
 * L3GD20.c
 *
 *  Created on: 10 paź 2020
 *      Author: Artur
 */

#include "L3GD20.h"

static SPI_HandleTypeDef *_handler;
static L3GD20_Sensitivity _sensitivity;

//private functions
static void L3GD20_Write(uint8_t*, uint8_t, uint16_t);
static void L3GD20_Read(uint8_t*, uint8_t, uint16_t);
static uint8_t L3GD20_WriteByte(uint8_t);

void L3GD20_Init(L3GD20_Sensitivity sensitivity, SPI_HandleTypeDef *handler) {

	_handler = handler;
	_sensitivity = sensitivity;

	uint8_t addr = 0, value = 0;

	addr = L3GD20_CTRL_REG1_ADDR;
	value = 0x3F;
	L3GD20_Write(&value, addr, 1);

	addr = L3GD20_CTRL_REG2_ADDR;
	value = 0x00;
	L3GD20_Write(&value, addr, 1);

	addr = L3GD20_CTRL_REG3_ADDR;
	value = 0x00;
	L3GD20_Write(&value, addr, 1);

	addr = L3GD20_CTRL_REG4_ADDR;
	value = 0x10;
	L3GD20_Write(&value, addr, 1);

	addr = L3GD20_CTRL_REG5_ADDR;
	value = 0x10;
	L3GD20_Write(&value, addr, 1);

}

void L3GD20_Reboot() {

	uint8_t tmpreg;
	L3GD20_Read(&tmpreg, L3GD20_CTRL_REG5_ADDR, 1);

	tmpreg |= 0x80;

	L3GD20_Write(&tmpreg, L3GD20_CTRL_REG5_ADDR, 1);

}

uint8_t L3GD20_ReadID() {

	uint8_t read = 0, addr = 0;

	addr = L3GD20_WHO_AM_I_ADDR;

	L3GD20_Read(&read, addr, 1);

	return read;
}

void L3GD20_ReadValues(L3GD20_Values *values) {
	uint8_t RawData[6];
	int16_t RawMeasurement[3];
	uint8_t tmpreg = 0, reg = 0;
	float sensitivity = 0.0f;
	int i = 0;

	reg = L3GD20_OUT_X_L_ADDR;
	L3GD20_Read(RawData, reg, 6);

	reg = L3GD20_CTRL_REG4_ADDR;
	L3GD20_Read(&tmpreg, reg, 1);

	if (!(tmpreg & L3GD20_BLE_MSB)) {
		for (i = 0; i < 3; i++) {
			RawMeasurement[i] = (int16_t) (((uint16_t) RawData[2 * i + 1] << 8)
					+ RawData[2 * i]);
		}
	} else {
		for (i = 0; i < 3; i++) {
			RawMeasurement[i] = (int16_t) (((uint16_t) RawData[2 * i] << 8)
					+ RawData[2 * i + 1]);
		}
	}


	if (_sensitivity == L3DS20_SENSITIVITY_250DPS) {
		sensitivity = L3GD20_SENSITIVITY_250DPS;
	} else if (_sensitivity == L3DS20_SENSITIVITY_500DPS) {
		sensitivity = L3GD20_SENSITIVITY_500DPS;
	} else if (_sensitivity == L3DS20_SENSITIVITY_2000DPS) {
		sensitivity = L3GD20_SENSITIVITY_2000DPS;
	}

	values->X = (float) (RawMeasurement[0] / sensitivity);
	values->Y = (float) (RawMeasurement[1] / sensitivity);
	values->Z = (float) (RawMeasurement[2] / sensitivity);

}

static void L3GD20_Write(uint8_t *pBuffer, uint8_t WriteAddr,
		uint16_t NumByteToWrite) {

	if (NumByteToWrite > 1)
		WriteAddr |= (uint8_t) 0x40;

	L3GD20_CS_LOW;
	L3GD20_WriteByte(WriteAddr);

	while (NumByteToWrite >= 0x01) {

		//HAL_SPI_TransmitReceive(_handler, pBuffer, read, NumByteToWrite, 100);
		L3GD20_WriteByte(*pBuffer);
		NumByteToWrite--;
		pBuffer++;

	}

	L3GD20_CS_HIGH;
}

static void L3GD20_Read(uint8_t *pBuffer, uint8_t readAddr,
		uint16_t NumByteToWrite) {

	if (NumByteToWrite > 1)
		readAddr |= (uint8_t) ((uint8_t) 0x80 | (uint8_t) 0x40);
	else
		readAddr |= (uint8_t) 0x80;

	L3GD20_CS_LOW;
	L3GD20_WriteByte(readAddr);

	while (NumByteToWrite > 0x00) {

		//HAL_SPI_TransmitReceive(_handler, pBuffer, read, NumByteToWrite, 100);
		*pBuffer = L3GD20_WriteByte((uint8_t) 0x00);
		NumByteToWrite--;
		pBuffer++;

	}

	L3GD20_CS_HIGH;

}

static uint8_t L3GD20_WriteByte(uint8_t reg) {

	uint8_t data = 0;

//	L3GD20_CS_LOW;

	HAL_SPI_TransmitReceive(_handler, (uint8_t*) &reg, (uint8_t*) &data, 1,
			100);

//	L3GD20_CS_HIGH;

	return data;
}
