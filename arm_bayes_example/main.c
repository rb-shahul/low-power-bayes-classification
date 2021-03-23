/* ----------------------------------------------------------------------
* Copyright (C) 2019-2020 ARM Limited. All rights reserved.
*
* $Date:         09. December 2019
* $Revision:     V1.0.0
*
* Project:       CMSIS DSP Library
* Title:         arm_bayes_example_f32.c
*
* Description:   Example code demonstrating how to use Bayes functions.
*
* Target Processor: Cortex-M/Cortex-A
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */


#include <math.h>
#include <stdio.h>
#include "arm_math.h"
#include "stm32l0xx_hal_conf.h"
#include "stm32l0xx_hal.h"
/*
Those parameters can be generated with the python library scikit-learn.
*/
arm_gaussian_naive_bayes_instance_f32 S;

#define NB_OF_CLASSES 3
#define VECTOR_DIMENSION 2

const float32_t theta[NB_OF_CLASSES * VECTOR_DIMENSION] = {
	1.4539529436590528f, 0.8722776016801852f,
	-1.5267934452462473f, 0.903204577814203f,
	-0.15338006360932258f, -2.9997913665803964f
}; /**< Mean values for the Gaussians */

const float32_t sigma[NB_OF_CLASSES * VECTOR_DIMENSION] = {
	1.0063470889514925f, 0.9038018246524426f,
	1.0224479953244736f, 0.7768764290432544f,
	1.1217662403241206f, 1.2303890106020325f
}; /**< Variances for the Gaussians */

const float32_t classPriors[NB_OF_CLASSES] = {
	0.3333333333333333f, 0.3333333333333333f, 0.3333333333333333f
}; /**< Class prior probabilities */


/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;


/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void MX_RTC_Init(void);
static void Enter_Sleep_Mode(void);
static void Disable_Gpios(void);
static void Stop_Mode_Resume(void);


int32_t main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_RTC_Init();
	/* Array of input data */
	float32_t in[2];

	/*All of these data taken CMSIS-DSP reference example 'arm_bayes_example' */
	/* Result of the classifier */
	float32_t result[NB_OF_CLASSES];
	float32_t maxProba;
	uint32_t index;

	S.vectorDimension = VECTOR_DIMENSION;
	S.numberOfClasses = NB_OF_CLASSES;
	S.theta = theta;
	S.sigma = sigma;
	S.classPriors = classPriors;
	S.epsilon = 4.328939296523643e-09f;

	in[0] = 1.5f;
	in[1] = 1.0f;

	while (1) {
		arm_gaussian_naive_bayes_predict_f32(&S, in, result);
		arm_max_f32(result, NB_OF_CLASSES, &maxProba, &index);
		Enter_Sleep_Mode();
		Stop_Mode_Resume();
	}
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{
	/** Initialize RTC Only
	*/
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                              | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

void Enter_Sleep_Mode()
{
// Configure RTC to wake up after 10ms
	uint32_t _time = (((uint32_t)10) * 2314) / 1000;
	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, _time, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
	__HAL_RCC_PWR_CLK_ENABLE(); // Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); // Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); // Fast wake-up for ultra low power mode

// Disable Unused Gpios
	Disable_Gpios(); // Disable GPIOs based on configuration

// GPIO Wake up is configured by CubeMx config and keeping the GPIO activated
// Switch to STOPMode
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void Disable_Gpios() {

	// GPIO_A is preserved to keep output status unchanged and have
	// Interrupt working for waking Up.


	GPIO_InitTypeDef GPIO_InitStructure = {0};
	GPIO_InitStructure.Pin = GPIO_PIN_All;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	// GPIO_A,GPIO_B,GPIO_C and GPIO_D are disabled
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	__HAL_RCC_GPIOA_CLK_DISABLE();
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	__HAL_RCC_GPIOB_CLK_DISABLE();
		HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	__HAL_RCC_GPIOC_CLK_DISABLE();
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	__HAL_RCC_GPIOD_CLK_DISABLE();

}
void Stop_Mode_Resume()
{
// Reinit clocks
	SystemClock_Config();
// Deactivate RTC wakeUp
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}