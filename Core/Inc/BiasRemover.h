/*
 * BiasRemover.h
 *
 *  Created on: 14 paź 2020
 *      Author: Artur
 */

#ifndef INC_BIASREMOVER_H_
#define INC_BIASREMOVER_H_

#include "L3GD20.h"
#include "LSM303DLHC.h"
#include "stdbool.h"

void BiasRemover_Init(int);
bool BiasRemover_Finished();
void BiasRemover_Get(LSM303DLHC_ACC_Values, LSM303DLHC_MAG_Values, L3GD20_Values);
void BiasRemover_Remove(LSM303DLHC_ACC_Values *, LSM303DLHC_MAG_Values *, L3GD20_Values *);
void BiasRemover_GetValues(LSM303DLHC_ACC_Values *, LSM303DLHC_MAG_Values *, L3GD20_Values *);


#endif /* INC_BIASREMOVER_H_ */
