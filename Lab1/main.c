/**
	SoC Architecture Lab 1 - Low Power Modes
	Robert Margelli - s224854
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f0_discovery.h"
#include "stm32f0xx_lp_modes.h"

/* Private variables ---------------------------------------------------------*/
__IO uint32_t i = 0;

/*
Implemented program:
	1. on first button press enter stop mode, exit after a time is elapsed (~5s);
	2. on button press enter sleep mode, exit by pressing user button;
	3. enter standby state and exits after a time is elapsed (~8s).
Notes:
	LEDs are used to distinguish intervals between low power states:
	  blue and green (initial state): run->stop
		green: stop->sleep
		blue: sleep->standby
*/

void Delay(long nCount);	// Delay function call eg.: Delay(0xFFFF);
void init_all();					// Initializes  USER BUTTON, GREEN and BLUE LEDs

int main(void)
{
		init_all();
	
    /* Enable PWR APB1 Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Allow access to Backup */
    PWR_BackupAccessCmd(ENABLE);
    /* Reset RTC Domain */
    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);

		while(STM_EVAL_PBGetState(BUTTON_USER) == SET);
	  /* Loop while User button has not been pressed */
		STM_EVAL_LEDOn(LED3);
		STM_EVAL_LEDOn(LED4);
		while(STM_EVAL_PBGetState(BUTTON_USER) == RESET);
		STM_EVAL_LEDOff(LED3);
		STM_EVAL_LEDOff(LED4);
		/* Enter STOP mode. Automatic Wakeup using RTC clocked by LSI (~5s) */
		StopMode_Measure();

		init_all();			// button and leds must be initialized after exiting a low power state
		/* Green LED on between STOP and SLEEP states */
		STM_EVAL_LEDOn(LED3);
		/* Loop while User button has not been pressed */
		while((STM_EVAL_PBGetState(BUTTON_USER) == RESET));// || time_elapsed())
		/* Turn off green LED on entering SLEEP state */
		STM_EVAL_LEDOff(LED3);
		/* IMPORTANT: because code is faster than human to go into sleep mode 
		and configure user button as external source of interrupt for wakeup */
		while((STM_EVAL_PBGetState(BUTTON_USER) == SET));	
		/* Enter SLEEP mode, Wakes up by using EXTI Line i.e. pressing the User Button PA.00 */		
		SleepMode_Measure();
		
		init_all();			// button and leds must be initialized after exiting a low power state
		/* Blue LED on between SLEEP and STANDBY states */
		STM_EVAL_LEDOn(LED4);
		/* Loop while User button has not been pressed */
		while((STM_EVAL_PBGetState(BUTTON_USER) == RESET));// || time_elapsed())
		/* IMPORTANT: because code is faster than human to go into standby mode 
		and configure user button as external source of interrupt for wakeup */
		while((STM_EVAL_PBGetState(BUTTON_USER) == SET));	
		/* Turn off blue LED on entering STANDBY state */
		STM_EVAL_LEDOff(LED4);
		
		/* Enter STANDBY mode. Wakeup using WakeUp Pin (PA.00).
		N.B. After waking up the MCU will be reset. */		
		StandbyRTCMode_Measure();
		
		/* Never reached as MCU is reset after resuming from STANDBY mode.*/
		while(1)
		{
			STM_EVAL_LEDToggle(LED4);
			Delay(0xFFFF);
			Delay(0xFFFF);
			Delay(0xFFFF);
			Delay(0xFFFF);
		}
  
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

void Delay(long nCount) {
	while (nCount !=0) {nCount--;}
}

void init_all() {
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	STM_EVAL_PBInit(BUTTON_USER,BUTTON_MODE_GPIO);	
}
