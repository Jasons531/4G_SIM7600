/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f2xx_hal.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

/* USER CODE BEGIN Includes */

#include "etimer.h"
#include "autostart.h"
#include "delay.h"
#include "sim7600.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
 /**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);  
  return ch;
}

uint16_t idle_counter = 0;

PROCESS(helloworld_process,"helloworld");
PROCESS(sim7600_break_process,"usartrx1");
PROCESS(sim7600_send_process,"usartrx2");

AUTOSTART_PROCESSES(&sim7600_send_process); //&helloworld_process,

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

process_event_t sim7600_callbreak;
process_event_t sim7600_call;


/*
*Function:helloworld Process
*Input Paragramed:
*Output Paragramed:
*Remarks:
*/
PROCESS_THREAD(helloworld_process,ev,data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System Blink Process..."); 
	etimer_set(&et,CLOCK_SECOND/1000);
	while(1)
	{	
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		HAL_UART_Receive_DMA(&huart1,(uint8_t *)UART_RX_DATA1.USART_RX_BUF,BUFFER_SIZE);

//		HAL_UART_Transmit_DMA(&huart1, "Hello world welcome\r\n", strlen("Hello world welcome\r\n"));
		etimer_reset(&et);
	}
	PROCESS_END();
}

/*
*Function:sim7600_break Process
*Input Paragramed:
*Output Paragramed:
*Remarks:
*/
PROCESS_THREAD(sim7600_break_process,ev,data)
{
	static struct etimer timer;
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System Usartrx Process..."); 

	etimer_set(&timer,CLOCK_SECOND);
	while(1)
	{	
	    //等待数据到来
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
//		if(UART_RX_DATA2.USART_RX_End_Flag)
		{
//			Usart1SendData_DMA(UART_RX_DATA2.USART_RX_BUF, UART_RX_DATA2.USART_RX_Len);
		
			if(UART_RX_DATA2.USART_TX_Flag == USART_DMA_SENDOVER)
			{
				memset(UART_RX_DATA2.USART_RX_BUF,0,UART_RX_DATA2.USART_RX_Len);
				UART_RX_DATA2.USART_RX_Len=0;
				UART_RX_DATA2.USART_RX_End_Flag = false;
//				process_post(&sim7600_send_process, sim7600_call, NULL);
			}
		}
		etimer_reset(&timer);
	}
	PROCESS_END();
}

bool first = false;

static uint32_t time_out = 0;

/*
*Function:sim7600_send Process
*Input Paragramed:
*Output Paragramed:
*Remarks:
*/
PROCESS_THREAD(sim7600_send_process,ev,data)
{
	static struct etimer timer;
	
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System Usartrx Process..."); 
	
	etimer_set(&timer,CLOCK_SECOND*20);

	while(1)
	{	
		PROCESS_YIELD();
		
		if(ev == PROCESS_EVENT_TIMER)
		{	
//			printf("----test----%d\r\n",HAL_GetTick()-time_out);
//			if(!first)
			{
				if(REPLY_SEND_DONE == SimcomSendData("hello world\r\n", 500)) ///发送成功，接收服务器应答，应答失败则再次重发两次，两次后不再重发
				first = true;			
			}

			///网络被断开，再次初始化网络
//			if(commandid_reply == REPLY_CIPCLOSE)
//			{
//				if(REPLY_CIPOPEN_ERROR == SimcomConnectServer(SERVER_ADDR, SERVER_PORT, 1000, 1))
//				{
//					printf("CIP is open\r\n");
//				}
//				first = false;
//				time_out = HAL_GetTick();
//			}
			etimer_reset(&timer);
		}
	}
	PROCESS_END();
}
/* USER CODE BEGIN PV */
 
/* USER CODE END PV */

/* USER CODE END 0 */

int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	delay_init(100);
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART1_UART_Init();
	MX_SPI3_Init();
	MX_SPI1_Init();
	MX_SPI2_Init();
	MX_USART2_UART_Init();

	/* Initialize interrupts */
	MX_NVIC_Init();

	/* USER CODE BEGIN 2 */
	///使能空闲中断
	HAL_UART_Receive_DMA(&huart1,(uint8_t *)UART_RX_DATA1.USART_RX_BUF,BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); 

	HAL_UART_Receive_DMA(&huart2,(uint8_t *)UART_RX_DATA2.USART_RX_BUF,BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); 

	uint8_t InitSimcomState = InitSimcom(  );

	if(InitSimcomState == INITSIMDONE)
	{
		DEBUG(2,"-----Init Simcom Done-----\r\n");
		SimcomOpenNet(  );
	
		SimcomConnectServer(  );
	}
	else
	{
		DEBUG(2,"-----Init Simcom Fail-----\r\n");
	}	
  
	clock_init();

	process_init();
	process_start(&etimer_process,NULL);
	autostart_start(autostart_processes);

	USR_UsrLog("System Contiki InitSuccess...");	
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	do
	{
	}while(process_run() > 0);
	idle_counter++; 

	}
	/* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** NVIC Configuration
*/
static void MX_NVIC_Init(void)
{
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 3, 3);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 3, 1);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 7, 1);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 7, 3);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* USER CODE BEGIN 4 */

extern DMA_HandleTypeDef hdma_usart1_rx;



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
