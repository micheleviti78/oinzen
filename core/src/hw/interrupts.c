/*===========================================================================
//
//        stm32f7xx_it.c
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

/* Includes ------------------------------------------------------------------*/
#include "interrupts.h"
#include "main.h"

extern TIM_HandleTypeDef htim6;
//extern ETH_HandleTypeDef heth;

/******************************************************************************/
/*           Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {}
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {}
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {}
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  while (1)
  {}
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  //HAL_IncTick();
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}

/**
  * @brief This function handles Ethernet global interrupt.
  */
void ETH_IRQHandler(void)
{
  //HAL_ETH_IRQHandler(&heth);
}
