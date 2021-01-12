/*===========================================================================
//
//        init.c
//
//===========================================================================
//===========================================================================
//
// oinzen project
//
//===========================================================================
//===========================================================================
//
// Author(s): michele
//
//===========================================================================*/

#include "stm32f7xx_hal_conf.h"

extern int _seth_descr_buffer;

void mpu_init(){

	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable MPU */
	HAL_MPU_Disable();

	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = (unsigned)(&_seth_descr_buffer);
	MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void hw_init(){
	mpu_init();
}
