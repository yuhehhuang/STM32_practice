//
// Created by yuheh on 2026/10/3.
//

#ifndef INTELLIGENT_TEMP_ALARM_AHT20_H
#define INTELLIGENT_TEMP_ALARM_AHT20_H

#include "i2c.h"
void AHT20_Init();

void AHT20_Read(float* Temperature,float* Humidity);
#endif //INTELLIGENT_TEMP_ALARM_AHT20_H
