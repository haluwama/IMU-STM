/*
 * LSM303DLHC.c
 *
 *  Created on: 13 paź 2020
 *      Author: Artur
 */

#include "LSM303DLHC.h"

I2C_HandleTypeDef *_i2c;
static void LSM303DLHC_Read(bool, uint16_t, uint8_t*, uint16_t);
static void LSM303DLHC_Write(uint16_t dev, uint8_t reg, uint8_t read);
static uint8_t LSM303DLHC_ReadByte(uint16_t, uint8_t);

void LSM303DLHC_Init(I2C_HandleTypeDef *i2c) {

	_i2c = i2c;

	//ACC initalization
	uint8_t settings;

	settings = LSM303DLHC_AXES_ENABLE | LSM303DLHC_ODR_100_HZ;
	LSM303DLHC_Write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG1_A, settings);

	settings = LSM303DLHC_HPM_NORMAL_MODE | LSM303DLHC_HPFCF_16;
	LSM303DLHC_Write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG2_A, settings);

	settings = LSM303DLHC_HR_ENABLE | LSM303DLHC_CONTINUOS_CONVERSION;
	LSM303DLHC_Write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG4_A, settings);

	//MAG initalization

	settings = LSM303DLHC_TEMPSENSOR_DISABLE | LSM303DLHC_ODR_220_HZ;
	LSM303DLHC_Write(MAG_I2C_ADDRESS, LSM303DLHC_CRA_REG_M, settings);

	settings = LSM303DLHC_FS_8_1_GA;
	LSM303DLHC_Write(MAG_I2C_ADDRESS, LSM303DLHC_CRB_REG_M, settings);

	settings = LSM303DLHC_CONTINUOS_CONVERSION;
	LSM303DLHC_Write(MAG_I2C_ADDRESS, LSM303DLHC_MR_REG_M, settings);

}

void LSM303DLHC_Reboot() {
	uint8_t settings;
	settings = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG5_A);
	settings |= 0x80;

	LSM303DLHC_Write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG5_A, settings);
}

uint8_t* LSM303DLHC_GetID() {
	static uint8_t arr[3] = { 0 };

	LSM303DLHC_Read(1, LSM303DLHC_IRA_REG_M, arr, 1);

	return arr;

}

void LSM303DLHC_GetValues(LSM303DLHC_ACC_Values *acc,
		LSM303DLHC_MAG_Values *mag) {
	int16_t pnRawData[3];
	uint8_t buffer[6];
	uint8_t i = 0, ctrlmag = 0;
	uint8_t ctrlx[2] = { 0, 0 };
	uint8_t sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_2G,
			sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_8_1Ga,
			sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_8_1Ga;

	ctrlx[0] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG4_A);
	ctrlx[1] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG5_A);

	buffer[0] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A);
	buffer[1] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_H_A);
	buffer[2] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_A);
	buffer[3] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_A);
	buffer[4] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_A);
	buffer[5] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_A);

	if (!(ctrlx[0] & 0x40)) {
		for (i = 0; i < 3; i++) {
			pnRawData[i] = ((int16_t) ((uint16_t) buffer[2 * i + 1] << 8)
					+ buffer[2 * i]);
		}
	} else {
		for (i = 0; i < 3; i++) {
			pnRawData[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
					+ buffer[2 * i + 1]);
		}
	}

	switch (ctrlx[0] & LSM303DLHC_FULLSCALE_16G) {
	case LSM303DLHC_FULLSCALE_2G:
		sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_2G;
		break;
	case LSM303DLHC_FULLSCALE_4G:
		sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_4G;
		break;
	case LSM303DLHC_FULLSCALE_8G:
		sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_8G;
		break;
	case LSM303DLHC_FULLSCALE_16G:
		sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_16G;
		break;
	}

	acc->X = (float) (pnRawData[0] * sensitivity_ACC) / (float) INT16_MAX;
	acc->Y = (float) (pnRawData[1] * sensitivity_ACC) / (float) INT16_MAX;
	acc->Z = (float) (pnRawData[2] * sensitivity_ACC) / (float) INT16_MAX;

	buffer[0] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_L_M);
	buffer[1] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_H_M);
	buffer[2] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_M);
	buffer[3] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_M);
	buffer[4] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_M);
	buffer[5] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_M);

	for (i = 0; i < 3; i++) {
		pnRawData[i] = ((int16_t) ((uint16_t) buffer[2 * i + 1] << 8)
				+ buffer[2 * i]);
	}

	ctrlmag = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_CRB_REG_M);

	switch (ctrlmag & LSM303DLHC_FS_8_1_GA) {
	case LSM303DLHC_FS_1_3_GA:
		sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_1_3Ga;
		sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_1_3Ga;
		break;
	case LSM303DLHC_FS_2_5_GA:
		sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_2_5Ga;
		sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_2_5Ga;
		break;
	case LSM303DLHC_FS_4_0_GA:
		sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_4Ga;
		sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_4Ga;
		break;
	case LSM303DLHC_FS_5_6_GA:
		sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_5_6Ga;
		sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_5_6Ga;
		break;
	case LSM303DLHC_FS_8_1_GA:
		sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_8_1Ga;
		sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_8_1Ga;
		break;
	}

	mag->X = (float) (pnRawData[0] * sensitivity_MAG_XY) / (float) INT16_MAX;
	mag->Y = (float) (pnRawData[1] * sensitivity_MAG_XY) / (float) INT16_MAX;
	mag->Z = (float) (pnRawData[2] * sensitivity_MAG_Z) / (float) INT16_MAX;
}

void LSM303DLHC_GetValues_Uint8_t(LSM303DLHC_ACC_Values_uint8_t *acc, LSM303DLHC_MAG_Values_uint8_t *mag)
{
	int16_t pnRawData[3];
		uint8_t buffer[6];
		uint8_t i = 0, ctrlmag = 0;
		uint8_t ctrlx[2] = { 0, 0 };
		uint8_t sensitivity_ACC = LSM303DLHC_ACC_SENSITIVITY_2G,
				sensitivity_MAG_XY = LSM303DLHC_M_SENSITIVITY_XY_8_1Ga,
				sensitivity_MAG_Z = LSM303DLHC_M_SENSITIVITY_Z_8_1Ga;

		ctrlx[0] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG4_A);
		ctrlx[1] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG5_A);

		buffer[0] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A);
		buffer[1] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_H_A);
		buffer[2] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_A);
		buffer[3] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_A);
		buffer[4] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_A);
		buffer[5] = LSM303DLHC_ReadByte(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_A);

		acc->X[0] = buffer[1];
		acc->X[1] = buffer[0];

		acc->Y[0] = buffer[3];
		acc->Y[1] = buffer[2];

		acc->Z[0] = buffer[5];
		acc->Z[1] = buffer[4];

		buffer[0] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_L_M);
		buffer[1] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_H_M);
		buffer[2] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_M);
		buffer[3] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_M);
		buffer[4] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_M);
		buffer[5] = LSM303DLHC_ReadByte(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_M);

		mag->X[0] = buffer[1];
		mag->X[1] = buffer[0];

		mag->Y[0] = buffer[3];
		mag->Y[1] = buffer[2];

		mag->Z[0] = buffer[5];
		mag->Z[1] = buffer[4];
}

static void LSM303DLHC_Read(bool dev, uint16_t reg, uint8_t *read,
		uint16_t NumByteToWrite) {

	if (dev == 0)
		HAL_I2C_Mem_Read(_i2c, ACC_I2C_ADDRESS, reg, 1, read, NumByteToWrite,
				100);
	else
		HAL_I2C_Mem_Read(_i2c, MAG_I2C_ADDRESS, reg, 1, read, NumByteToWrite,
				100);

}

static void LSM303DLHC_Write(uint16_t dev, uint8_t reg, uint8_t read) {

	HAL_I2C_Mem_Write(_i2c, dev, reg, 1, &read, 1, 100);

}

static uint8_t LSM303DLHC_ReadByte(uint16_t dev, uint8_t reg) {

	uint8_t settings = 0;

	HAL_I2C_Mem_Read(_i2c, dev, reg, 1, &settings, 1, 100);

	return settings;
}
