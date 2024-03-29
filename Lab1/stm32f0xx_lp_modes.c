/**
	SoC Architecture Lab 1 - Low Power Modes
	Robert Margelli - s224854
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_lp_modes.h"
#include "stm32f0_discovery.h"

// #define ULTRA_LOWPOWER	// comment this line to test normal low power modes

/**
  * @brief  This function configures the system to enter Sleep mode for
  *         current consumption measurement purpose.
  *         Sleep Mode
  *         ==========  
  *            - System Running at PLL (48MHz)
  *            - Flash 3 wait state
  *            - Prefetch and Cache enabled
  *            - Code running from Internal FLASH
  *            - All peripherals disabled.
  *            - Wakeup using EXTI Line (User Button PA.00)
  * @param  None
  * @retval None
  */
void SleepMode_Measure(void)
{
  __IO uint32_t index = 0;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOF , ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                         RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOF, DISABLE);
		

  /* Configure User Button */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Request to enter SLEEP mode */
  __WFI();
}

/**
  * @brief  This function configures the system to enter Stop mode with RTC 
  *         clocked by LSI  for current consumption measurement purpose.
  *         STOP Mode with RTC clocked by LSI
  *         =====================================   
  *           - RTC Clocked by LSI
  *           - Regulator in LP mode
  *           - HSI, HSE OFF and LSI OFF if not used as RTC Clock source
  *           - No IWDG
  *           - FLASH in deep power down mode
  *           - Automatic Wakeup using RTC clocked by LSI (~5s)
  * @param  None
  * @retval None
  */
void StopMode_Measure(void)
{
  __IO uint32_t index = 0;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;
  RTC_InitTypeDef   RTC_InitStructure;
  RTC_TimeTypeDef   RTC_TimeStructure;
  RTC_AlarmTypeDef  RTC_AlarmStructure;
  
  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);

/* The RTC Clock may varies due to LSI frequency dispersion. */   
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);

  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                         RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOF , ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA |RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                         RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOF, DISABLE);
 
 // extra disables
 #ifdef ULTRA_LOWPOWER
 /* Available AHB peripherals that can be turned off, those marked with 'NO' have not been switched off
   *             @arg RCC_AHBPeriph_GPIOA:         GPIOA clock
  *             @arg RCC_AHBPeriph_GPIOB:         GPIOB clock
  *             @arg RCC_AHBPeriph_GPIOC:         GPIOC clock
  *             @arg RCC_AHBPeriph_GPIOD:         GPIOD clock
  *             @arg RCC_AHBPeriph_GPIOF:         GPIOF clock
  *             @arg RCC_AHBPeriph_TS:            TS clock				// NO
  *             @arg RCC_AHBPeriph_CRC:           CRC clock
  *             @arg RCC_AHBPeriph_FLITF: (has effect only when the Flash memory is in power down mode)  	// NO
  *             @arg RCC_AHBPeriph_SRAM:          SRAM clock			// NO
  *             @arg RCC_AHBPeriph_DMA1:          DMA1 clock
*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC | RCC_AHBPeriph_DMA1, DISABLE);
 /* Available APB2 peripherals that can be turned off, those marked with 'NO' have not been switched off
  *             @arg RCC_APB2Periph_SYSCFG:      SYSCFG clock			// NO (configures system registers)
  *             @arg RCC_APB2Periph_ADC1:        ADC1 clock
  *             @arg RCC_APB2Periph_TIM1:        TIM1 clock
  *             @arg RCC_APB2Periph_SPI1:        SPI1 clock
  *             @arg RCC_APB2Periph_USART1:      USART1 clock
  *             @arg RCC_APB2Periph_TIM15:       TIM15 clock
  *             @arg RCC_APB2Periph_TIM16:       TIM16 clock
  *             @arg RCC_APB2Periph_TIM17:       TIM17 clock
  *             @arg RCC_APB2Periph_DBGMCU:      DBGMCU clock
*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1 | RCC_APB2Periph_SPI1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_TIM15 | RCC_APB2Periph_TIM16 | RCC_APB2Periph_TIM17 | RCC_APB2Periph_DBGMCU, DISABLE);
 /* Available APB1 peripherals that can be turned off, those marked with 'NO' have not been switched off
   *           @arg RCC_APB1Periph_TIM2:      TIM2 clock
  *           @arg RCC_APB1Periph_TIM3:      TIM3 clock
  *           @arg RCC_APB1Periph_TIM6:      TIM6 clock
  *           @arg RCC_APB1Periph_TIM14:     TIM14 clock
  *           @arg RCC_APB1Periph_WWDG:      WWDG clock			
  *           @arg RCC_APB1Periph_SPI2:      SPI2 clock
  *           @arg RCC_APB1Periph_USART2:    USART2 clock
  *           @arg RCC_APB1Periph_I2C1:      I2C1 clock
  *           @arg RCC_APB1Periph_I2C2:      I2C2 clock
  *           @arg RCC_APB1Periph_PWR:       PWR clock			// NO
  *           @arg RCC_APB1Periph_DAC:       DAC clock
  *           @arg RCC_APB1Periph_CEC:       CEC clock  		
 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM6 | RCC_APB1Periph_TIM14 | RCC_APB1Periph_WWDG | RCC_APB1Periph_SPI2 | RCC_APB1Periph_USART2 | RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2 | RCC_APB1Periph_DAC | RCC_APB1Periph_CEC, DISABLE);
#endif
 
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv = 0x0138;
  
  if (RTC_Init(&RTC_InitStructure) == ERROR)
  {
    while(1);
  }
    
  /* EXTI configuration */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* NVIC configuration */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Set the alarm X+5s */
  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x01;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x00;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x05;	// MV
  RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
  RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
  RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
  RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
  
  /* Enable the alarm */
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
  
  /* Enable the RTC Alarm A interrupt */
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
  
  /* Set the time to 01h 00mn 00s AM */
  RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
  RTC_TimeStructure.RTC_Hours   = 0x01;
  RTC_TimeStructure.RTC_Minutes = 0x00;
  RTC_TimeStructure.RTC_Seconds = 0x00;  
  
  RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
  
  /* Clear the Alarm A Pending Bit */
  RTC_ClearITPendingBit(RTC_IT_ALRA);  

  /* Enter Stop Mode */
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

}

/**
  * @brief  This function configures the system to enter Standby mode for
  *         current consumption measurement purpose.
  *         STANDBY Mode
  *         ============
  *           - Backup SRAM and RTC OFF
  *           - IWDG and LSI OFF
  *           - Wakeup using WakeUp Pin (PC.13)
  * @param  None
  * @retval None
  */
void StandbyMode_Measure(void)
{
  /* Enable WKUP pin 2 */
  PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE);

  /* Request to enter STANDBY mode (Wake Up flag is cleared in PWR_EnterSTANDBYMode function) */
  PWR_EnterSTANDBYMode();
  
  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  This function configures the system to enter Standby mode with RTC 
  *         clocked by LSI for current consumption measurement purpose.
  *         STANDBY Mode with RTC clocked by LSI
  *         ========================================
  *           - RTC Clocked by LSI
  *           - IWDG OFF
  *           - Automatic Wakeup using RTC clocked by LSI 
  * @param  None
  * @retval None
  */
void StandbyRTCMode_Measure(void)
{
  RTC_InitTypeDef   RTC_InitStructure;
  RTC_AlarmTypeDef  RTC_AlarmStructure;
  RTC_TimeTypeDef   RTC_TimeStructure;
  
  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);
  
  /* The RTC Clock may varies due to LSI frequency dispersion. */   
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }
  
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
  
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);
  
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
  
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv = 0x0138;
  
  RTC_Init(&RTC_InitStructure);
  
  /* Set the alarm X+5s */
  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x01;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x00;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x08;
  RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
  RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
  RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
  RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
  
  /* Enable RTC Alarm A Interrupt */
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
  
  /* Enable the alarm */
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
  
  /* Set the time to 01h 00mn 00s AM */
  RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
  RTC_TimeStructure.RTC_Hours   = 0x01;
  RTC_TimeStructure.RTC_Minutes = 0x00;
  RTC_TimeStructure.RTC_Seconds = 0x00;  
  
  RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
    
  /* Clear Wakeup flag */
  PWR_ClearFlag(PWR_FLAG_WU);      
  
  RTC_ClearFlag(RTC_FLAG_ALRAF);
  
  /* Request to enter STANDBY mode (Wake Up flag is cleared in PWR_EnterSTANDBYMode function) */
  PWR_EnterSTANDBYMode();

}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/