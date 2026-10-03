//
// Created by yuheh on 2026/10/3.
//
#include "aht20.h"

#include "cmsis_os2.h"

#define AHT20_ADDRESS 0x70

extern osMutexId_t I2C_MutexHandle;
 void AHT20_Init()
 {
     uint8_t readBuffer;
     osDelay((40));//通電後要等待40ms;
     HAL_I2C_Master_Receive(&hi2c1,AHT20_ADDRESS,&readBuffer,1,HAL_MAX_DELAY);
     if ((readBuffer&0x08)==0x00)
     {
         uint8_t sendBuffer[3]={0xBE,0x08,0x00};
         HAL_I2C_Master_Transmit(&hi2c1,AHT20_ADDRESS,sendBuffer,3,HAL_MAX_DELAY);
     }
 };

void AHT20_Read(float* Temperature,float* Humidity)
{
    uint8_t sendBuffer[3]={0xAC,0x33,0x00};
    uint8_t readBuffer[6];
    osSemaphoreAcquire(I2C_MutexHandle, osWaitForever);
    //1.主機傳送蒐集資料任務命令
    if (HAL_I2C_Master_Transmit(&hi2c1,AHT20_ADDRESS,sendBuffer,3,100)!=HAL_OK)
    {
        osSemaphoreRelease(I2C_MutexHandle);
        return;
    }
    osSemaphoreRelease(I2C_MutexHandle);
    //2.收集資料
    osDelay((75));
    //3.把資料寫進buffer
    osSemaphoreAcquire(I2C_MutexHandle, osWaitForever);
    HAL_StatusTypeDef status=HAL_I2C_Master_Receive(&hi2c1,AHT20_ADDRESS,readBuffer,6,HAL_MAX_DELAY);
    osSemaphoreRelease(I2C_MutexHandle);

    if (status==HAL_OK&&(readBuffer[0]&0x80)==0x00)//正確讀取
    {
        uint32_t data=0;
        data=((uint32_t)readBuffer[3]>>4)+((uint32_t)readBuffer[2]<<4)+((uint32_t)readBuffer[1]<<12);//濕度數據
        *Humidity=data*100.0f/(1<<20);

        data=(((uint32_t)readBuffer[3]&0x0F)<<16)+((uint32_t)readBuffer[4]<<8)+(uint32_t)readBuffer[5];
        *Temperature=data*200.0f/(1<<20)-50;
    }
}