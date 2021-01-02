/*===========================================================================
//
//        stm32f7xx_it.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void TIM6_DAC_IRQHandler(void);
void ETH_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_IT_H */
