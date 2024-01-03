/**
  ******************************************************************************
	* Robert Margelli - 224854
  *	System-on-chip architecture LAB 2
	*
	* After initializations, the program while(1) loop's behaviour is as follows:
	* 1) DAC sends out selected waveform;
	* 2) at the end of the waveform DMA sends interrupt which triggers StandbyRTC mode;
	* 3) before going into StandbyRTC mode save the current value of SelectedWaveform in the RTC_BKP_DR0 backup register;
	* 4) automatic wakeup after 1 second and reset the system -> start over and restore the value of SelectedWaveForms that is in RTC_BKP_DR0.
	*
	* LEDs indicate which stage we are into:
	* 1) green: sinewave
	* 2) blue: escalator wave
	* 3) green (again i.e. green after blue): "square wave", pulse
	* 4) green and blue: triangle wave
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "stm32f0xx_lp_modes.h"

/* Private define ------------------------------------------------------------*/
#define DAC_DHR12R1_ADDRESS      0x40007408
#define DAC_DHR8R1_ADDRESS       0x40007410

/* Private variables ---------------------------------------------------------*/
TIM_TimeBaseInitTypeDef   	TIM_TimeBaseStructure;
DAC_InitTypeDef           	DAC_InitStructure;
DMA_InitTypeDef            	DMA_InitStructure;

/* Waveform definitions: sinewave, escalator, square, triangle */
const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};	  
const uint8_t Escalator8bit[6] = {0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF};	  
const uint8_t Square8bit[6] = {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};
const uint16_t Triangle16bit[31] = {0x0000, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF, 0xEEEE, 0xDDDD, 0xCCCC, 0xBBBB, 0xAAAA, 0x9999, 0x8888, 0x7777, 0x6666, 0x5555, 0x4444, 0x3333, 0x2222, 0x1111, 0x0000};
__IO uint8_t SelectedWavesForm = 0;
__IO uint8_t WaveChange = 1; 

/* Private functions ---------------------------------------------------------*/
void DAC_Config(void);
void configureNVICforDMA(void);
	
/* main() */
int main(void)
{
  /*! At this stage the microcontroller clock setting is already configured, 
      this is done through SystemInit() function which is called from startup
      file (startup_stm32f0xx.s) before to branch to application main.
      To reconfigure the default setting of SystemInit() function, refer to
      system_stm32f0xx.c file
  */ 
	 
  /* Preconfiguration before using DAC----------------------------------------*/
  DAC_Config();
  
  /* TIM2 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 0xE4EB2;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0x3; //3;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* TIM2 TRGO selection */
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
  
  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable the backup registers and read the last value stored in RTC_BKP_DR0.
		 This value is initially set to 0 because enabling its access resets its content.
		 This way SelectedWavesForm has the value of 0 (sinewave) at the first run. */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	SelectedWavesForm = RTC_ReadBackupRegister(RTC_BKP_DR0);
	
	/* Configures Button GPIO and EXTI Line */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
	
	/* Enables green and blue LEDs.  Used for letting the user understand which wave is being emitted. */
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	
  /* Infinite loop */
  while (1)
  {
    /* If the wave form is changed */
    if (WaveChange == 1)
    {  
      /* Switch the selected waves forms according the Button status */
      if (SelectedWavesForm == 0)
      {
          /* The sine wave has been selected */
          /* Sine Wave generator ---------------------------------------------*/
          DAC_DeInit(); 
          
          /* DAC channel1 Configuration */
          DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
          DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
   			
          /* DMA channel3 Configuration */
          DMA_DeInit(DMA1_Channel3);	/* de-initialize DMA channel 3 */
          DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS;
          DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Sine12bit;
				  DMA_InitStructure.DMA_BufferSize = 32;
				  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
          DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
				
          DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
          DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
          DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
          DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
          DMA_InitStructure.DMA_Priority = DMA_Priority_High;
          DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
					
					DMA_Init(DMA1_Channel3, &DMA_InitStructure);
					DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);  /* enable external interrupts from the DMA channel 3  */
		  
					/*configure NVIC for DMA channel 3 */
					configureNVICforDMA();		

          /* Enable DMA1 Channel3 */
          DMA_Cmd(DMA1_Channel3, ENABLE);

          /* DAC Channel1 Init */
          DAC_Init(DAC_Channel_1, &DAC_InitStructure);
		  
          /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
             automatically connected to the DAC converter. */
          DAC_Cmd(DAC_Channel_1, ENABLE);

          /* Enable DMA for DAC Channel1 */
          DAC_DMACmd(DAC_Channel_1, ENABLE);
  
					/* State is GREEN: Sinewave */
					STM_EVAL_LEDOff(LED4);
					STM_EVAL_LEDOn(LED3);
			}
			
          /* The Escalator wave has been selected */
      else if (SelectedWavesForm == 1) {
          /* Escalator Wave generator -----------------------------------------*/
          DAC_DeInit();
          
          /* DAC channel1 Configuration */
          DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
          DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
 
 
          /* DMA1 channel2 configuration */
          DMA_DeInit(DMA1_Channel3);	/* de-initialize DMA channel 3 */
          DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;
          DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Escalator8bit;
          DMA_InitStructure.DMA_BufferSize = 6;
          DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
          DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
				
					DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
          DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
          DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
          DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
          DMA_InitStructure.DMA_Priority = DMA_Priority_High;
          DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
					
					DMA_Init(DMA1_Channel3, &DMA_InitStructure);
					DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);  /* enable external interrupts from the DMA channel 3  */
			
					/*configure NVIC for DMA channel 3 */
					configureNVICforDMA();

          /* Enable DMA1 Channel3 */
          DMA_Cmd(DMA1_Channel3, ENABLE);

          /* DAC channel1 Configuration */
          DAC_Init(DAC_Channel_1, &DAC_InitStructure);

          /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
             automatically connected to the DAC converter. */
          DAC_Cmd(DAC_Channel_1, ENABLE);

          /* Enable DMA for DAC Channel1 */
          DAC_DMACmd(DAC_Channel_1, ENABLE);

					/* State is BLUE: Escalator wave */
					STM_EVAL_LEDOff(LED3);
					STM_EVAL_LEDOn(LED4);
        }
		
			else if (SelectedWavesForm == 2){
        /* Square wave generator -----------------------------------------*/
        DAC_DeInit();
        
        /* DAC channel1 Configuration */
        DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
        DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
        /* DMA1 channel3 configuration */
        DMA_DeInit(DMA1_Channel3);	/* de-initialize DMA channel 3 */
        DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Square8bit;
        DMA_InitStructure.DMA_BufferSize = 6;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_High;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        
        DMA_Init(DMA1_Channel3, &DMA_InitStructure);
        DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);  /* enable external interrupts from the DMA channel 3  */
    
    
        /*configure NVIC for DMA channel 3 */
        configureNVICforDMA();

        /* Enable DMA1 Channel2 */
        DMA_Cmd(DMA1_Channel3, ENABLE);

        /* DAC channel1 Configuration */
        DAC_Init(DAC_Channel_1, &DAC_InitStructure);

        /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
            automatically connected to the DAC converter. */
        DAC_Cmd(DAC_Channel_1, ENABLE);

        /* Enable DMA for DAC Channel1 */
        DAC_DMACmd(DAC_Channel_1, ENABLE);

        /* State is GREEN, again: Squarewave*/
        STM_EVAL_LEDOff(LED4);
        STM_EVAL_LEDOn(LED3);
      }
	
				else if (SelectedWavesForm == 3){
          /* triangle wave generator -----------------------------------------*/
          DAC_DeInit();
          
          /* DAC channel1 Configuration */
          DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
          DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
					/* DMA1 channel3 configuration */
          DMA_DeInit(DMA1_Channel3);	/* de-initialize DMA channel 3 */
					DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS;
          DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Triangle16bit;
          DMA_InitStructure.DMA_BufferSize = 31;
          DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
          DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
					DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
          DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
          DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
          DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
          DMA_InitStructure.DMA_Priority = DMA_Priority_High;
					DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
					
					DMA_Init(DMA1_Channel3, &DMA_InitStructure);
					DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);  /* enable external interrupts from the DMA channel 3  */
			
					/*configure NVIC for DMA channel 3 */
					configureNVICforDMA();

          /* Enable DMA1 Channel3 */
          DMA_Cmd(DMA1_Channel3, ENABLE);

          /* DAC channel1 Configuration */
          DAC_Init(DAC_Channel_1, &DAC_InitStructure);

          /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
             automatically connected to the DAC converter. */
          DAC_Cmd(DAC_Channel_1, ENABLE);

          /* Enable DMA for DAC Channel1 */
          DAC_DMACmd(DAC_Channel_1, ENABLE);
					
					/* State is GREEN and BLUE, last waveform: Triangle wave*/
					STM_EVAL_LEDOn(LED4);
					STM_EVAL_LEDOn(LED3);
        }
	     WaveChange = !WaveChange;
    }
	} /* end of while(1) loop */
} /* end of main () */


/**
  * @brief  PrecConfiguration: configure PA4 in analog,
  *                           enable DAC clock, enable DMA1 clock
  * @param  None
  * @retval None
  */
void DAC_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* DMA1 clock enable (to be used with DAC) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* Configure PA.04 (DAC_OUT1) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void configureNVICforDMA(void){
					NVIC_InitTypeDef NVIC_InitStructure;
					NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
					NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
					NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
					NVIC_Init(&NVIC_InitStructure);	
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
 // while (1)
 // {
 // }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
