# low-power-bayes-classification

## Problem statement:
Write a program targeting any STM32 Cortex M0+ microcontroller to perform a Naive Bayes classification using CMSIS-DSP, every 10ms.
Any data can be taken for this. The firmware should be written to exhibit minimal power consumption without performance loss.
Please make sure to include your justifications for the program routine, clock configurations, power consumption, etc. in a README folder in markdown.

#### MCU:STM32L010RB

#### Design consideration: 
Req->Need RAM Retention , Wakeup source for every 10 ms and low power.
Sol->Stop mode with RTC, clocked by LSI @ 37kHz.
->CPU core can be clocked by HSI@16Mhz.

#### Datas:
->Down to 93 µA/MHz in Run mode.
->Max Current Cunsumption in Stop mode with RTC is : 2.5 uA
->Stop Mode with RTC :In Stop Mode with RTC, the CPU core is stopped but the RAM,RTC and Register are retained.
->Wake-up time: 5uS.
->Wake-up signal is from RTC… Current is down to 0.8uA @ 3V

#### Skelton of program:
```
int32_t main(void)
{
	Init_System();
	Initialise_Gaussian_Params();
	while (1) {
		arm_gaussian_naive_bayes_predict_f32(&S, in, result);
		arm_max_f32(result, NB_OF_CLASSES, &maxProba, &index);
		Enter_Sleep_Mode();
		//will wake up after 10 ms
		Stop_Mode_Resume();
	}
}
```
Before going to sleep mode device is configured to following 
->RTC Wakeup time
we are using 37Khz/16 LSI clock for RTC.This allows a 2314 RTC ticks / s.
```
void Enter_Sleep_Mode()
{
// Configure RTC to wake up after 10ms
	uint32_t _time = (((uint32_t)10) * 2314) / 1000;
	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, _time, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
	__HAL_RCC_PWR_CLK_ENABLE(); // Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); // Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); // Fast wake-up for ultra low power mode

// Disable Unused Gpios to avoid leakage current
	Disable_Gpios(); // Disable GPIOs based on configuration
// Switch to STOPMode
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
```
After waking up again configure system clock and de activate rtc wakeup timer.
```
void Stop_Mode_Resume()
{
// Reinit clocks
	SystemClock_Config();
// Deactivate RTC wakeUp
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}
```


**Note:** I have used example code from CMSIS-DAP lib as reference for both data and program.