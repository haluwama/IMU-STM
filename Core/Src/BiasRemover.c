/*
 * BiasRemover.c
 *
 *  Created on: 14 paź 2020
 *      Author: Artur
 */

#include "BiasRemover.h"

static int _maxSamples;
static int _currentSamples;

static L3GD20_Values _gyro;
static LSM303DLHC_ACC_Values _acc;
static LSM303DLHC_MAG_Values _mag;

void BiasRemover_Init(int max_samples)
{
	_maxSamples = max_samples;

	_acc.X = 0;
	_acc.Y = 0;
	_acc.Z = 0;

	_mag.X = 0;
	_mag.Y = 0;
	_mag.Z = 0;

	_gyro.X = 0;
	_gyro.Y = 0;
	_gyro.Z = 0;
}

bool BiasRemover_Finished()
{

	if (_maxSamples == _currentSamples)
		return true;
	else
		return false;

}

void BiasRemover_Get(LSM303DLHC_ACC_Values acc, LSM303DLHC_MAG_Values mag,
		L3GD20_Values gyro)
{

	if (_currentSamples == 0)
	{
		_acc = acc;
		_mag = mag;
		_gyro = gyro;

		_currentSamples++;
	} else if (_currentSamples <= _maxSamples)
	{
		_acc.X = (acc.X + _acc.X) / 2;
		_acc.Y = (acc.Y + _acc.Y) / 2;
		_acc.Z = (acc.Z + _acc.Z) / 2;

		_mag.X = (mag.X + _mag.X) / 2;
		_mag.Y = (mag.Y + _mag.Y) / 2;
		_mag.Z = (mag.Z + _mag.Z) / 2;

		_gyro.X = (gyro.X + _gyro.X) / 2;
		_gyro.Y = (gyro.Y + _gyro.Y) / 2;
		_gyro.Z = (gyro.Z + _gyro.Z) / 2;

		_currentSamples++;
	}

}

void BiasRemover_Remove(LSM303DLHC_ACC_Values *acc, LSM303DLHC_MAG_Values *mag, L3GD20_Values *gyro)
{

	acc->X = acc->X - _acc.X;
	acc->Y = acc->Y - _acc.Y;
	acc->Z = acc->Z - _acc.Z;

	mag->X = mag->X - _mag.X;
	mag->Y = mag->Y - _mag.Y;
	mag->Z = mag->Z - _mag.Z;

	gyro->X = gyro->X - _gyro.X;
	gyro->Y = gyro->Y - _gyro.Y;
	gyro->Z = gyro->Z - _gyro.Z;

}

void BiasRemover_GetValues(LSM303DLHC_ACC_Values *acc, LSM303DLHC_MAG_Values *mag, L3GD20_Values *gyro)
{
	(*acc) = _acc;
	(*mag) = _mag;
	(*gyro) = _gyro;
}
