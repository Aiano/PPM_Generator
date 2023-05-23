/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PPM_CHANNEL_NUM 8
#define PPM_WAVE_NUM (PPM_CHANNEL_NUM + 1)*2
#define PPM_CHANNEL_HEADER_WIDTH 400
#define PPM_MAX_WIDTH 19999
#define PPM_CHANNEL_MIN_WIDTH 1000
#define PPM_CHANNEL_MAX_WIDTH 2000

#define RX_PIN GPIO_PIN_8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Rx buffer
uint16_t rx_ppm_pulse_width_us[PPM_CHANNEL_NUM] = {1500, 1500, 1000, 1500, 1000, 1000, 1500, 1500}; // pulse width involve header

// Tx buffer
uint16_t ppm_pulse_width_us[PPM_CHANNEL_NUM] = {1500, 1500, 1000, 1500, 1000, 1000, 1500, 1500}; // pulse width involve header
uint16_t ppm_wave[PPM_WAVE_NUM];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ppm_process();
void ppm_set_channel_values_6chs(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4, uint16_t ch5, uint16_t ch6);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	ppm_process();
	HAL_TIM_OC_Start_DMA(&htim3, TIM_CHANNEL_3, (uint32_t *)ppm_wave, PPM_WAVE_NUM);
	HAL_TIM_Base_Start(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		for(int i = 0; i < PPM_CHANNEL_NUM; i++){
			ppm_pulse_width_us[i] = rx_ppm_pulse_width_us[i];
		}
		ppm_process();
		
		if(rx_ppm_pulse_width_us[5] > 1800){
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		}else{
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}
		HAL_Delay(20);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void ppm_process(){
	ppm_wave[0] = PPM_CHANNEL_HEADER_WIDTH;
	ppm_wave[1] = ppm_pulse_width_us[0];
	for(int i = 1; i< PPM_CHANNEL_NUM; i++){
		ppm_wave[i*2] = ppm_wave[i*2-1] + PPM_CHANNEL_HEADER_WIDTH;
		ppm_wave[i*2+1] = ppm_wave[i*2-1] + ppm_pulse_width_us[i];
	}
	ppm_wave[PPM_WAVE_NUM - 2] = ppm_wave[PPM_WAVE_NUM - 3] + PPM_CHANNEL_HEADER_WIDTH;
	ppm_wave[PPM_WAVE_NUM - 1] = PPM_MAX_WIDTH;
}


uint8_t channel_index = 0;
uint16_t pulse_width = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){ // Detect falling edge
	if(GPIO_Pin == RX_PIN){
		pulse_width = __HAL_TIM_GET_COUNTER(&htim4); // Get timer counts (us)
		__HAL_TIM_SET_COUNTER(&htim4, 0); // Clear timer counts
		
		if(pulse_width < PPM_CHANNEL_MIN_WIDTH - 200){ // Invalid value
			channel_index = 0;
		}else if(pulse_width > PPM_CHANNEL_MAX_WIDTH + 200){ // Next frame
			channel_index = 0;
		}else if(channel_index < PPM_CHANNEL_NUM){ // Valid value
			rx_ppm_pulse_width_us[channel_index++] = pulse_width;
		}
	}

}

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
