/*
 * aht20.h
 *
 *  Created on: Aug 8, 2026
 *      Author: yuheh
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_


#include "i2c.h"
void AHT20_Init();
void AHT20_Read(float* Temperature, float* Humidity);
#endif /* INC_AHT20_H_ */
