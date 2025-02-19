/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "RM3100.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

uint8_t Continuous_Measurement_Mode_Value;
uint8_t RM3100_Init_Status;

uint8_t Read_Measurement_Start_Address = 0xA4;
uint8_t buffer[9];

uint8_t Status_Read_Register_Address = 0xB4;
uint8_t RM3100_status;

int32_t Mx, My, Mz;

float Mx_uT, My_uT, Mz_uT;


static RM3100_ RM3100;

char sendData[100];

float heading;
float heading_cal;
float magnetic_declination = -9.76;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t Check_RM3100_Status()
{
	uint8_t status;
	uint8_t data;
	uint8_t data1;
	uint8_t status_register = 0xB4;

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &status_register, 1, 100);
	HAL_SPI_Receive(&hspi1, &data, 1, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);
	data1 = 0x80 & data;

	if (data1 == 0x00)
	{
		status = 0;
	}
	if (data1 == 0x80)
	{
		status = 1;
	}

	return status;
}

uint8_t RM3100_Init(uint8_t Continuous_Mode_value, uint8_t Update_Rate_Value, uint8_t Cycle_Count_Value)
{
	uint8_t RM3100_Init_Result;

	uint8_t CCX_MSB = 0x04;
	uint8_t CCX_MSB_Address_Read = 0x80 | CCX_MSB;
	uint8_t CC_Buffer[6] = { 0, Cycle_Count_Value, 0, Cycle_Count_Value, 0, Cycle_Count_Value };
	uint8_t CC_Buffer_Read[6];
	uint8_t Cycle_flag = 0;

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &CCX_MSB, 1, 100);
	HAL_SPI_Transmit(&hspi1, CC_Buffer, 6, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &CCX_MSB_Address_Read, 1, 100);
	HAL_SPI_Receive(&hspi1, CC_Buffer_Read, 6, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	if (CC_Buffer_Read[1] == CC_Buffer[1])
	{
		Cycle_flag = 1;
	}
	else
	{
		Cycle_flag = 0;
	}

	uint8_t Continuous_Measurement_Mode_Address = RM3100_CMM;
	uint8_t Continuous_Measurement_Mode_Read_Address = (0x80 | RM3100_CMM);
	uint8_t Continuous_Measurement_Mode_Value = Continuous_Mode_value;
	uint8_t Continuous_Measurement_Mode_Read_Value = 0;
	uint8_t CMM_flag = 0;

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &Continuous_Measurement_Mode_Address, 1, 100);
	HAL_SPI_Transmit(&hspi1, &Continuous_Measurement_Mode_Value, 1, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &Continuous_Measurement_Mode_Read_Address, 1, 100);
	HAL_SPI_Receive(&hspi1, &Continuous_Measurement_Mode_Read_Value, 1, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	if (Continuous_Measurement_Mode_Read_Value == Continuous_Measurement_Mode_Value)
	{
		CMM_flag = 1;
	}
	else
	{
		CMM_flag = 0;
	}

	uint8_t TMRC_Address = RM3100_TMRC;
	uint8_t TMRC_Address_Read = 0x80 | RM3100_TMRC;
	uint8_t TMRC_Value = Update_Rate_Value;
	uint8_t TMRC_Value_Read;
	uint8_t TMRC_flag = 0;

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &TMRC_Address, 1, 100);
	HAL_SPI_Transmit(&hspi1, &TMRC_Value, 1, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
	HAL_SPI_Transmit(&hspi1, &TMRC_Address_Read, 1, 100);
	HAL_SPI_Receive(&hspi1, &TMRC_Value_Read, 1, 100);
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);

	if (TMRC_Value_Read == TMRC_Value)
	{
		TMRC_flag = 1;
	}
	else
	{
		TMRC_flag = 0;
	}

	if (Cycle_flag == 1 && CMM_flag == 1 && TMRC_flag == 1)
	{
		RM3100_Init_Result = 1;
	}
	else
	{
		RM3100_Init_Result = 0;
	}

	return RM3100_Init_Result;

}

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
	MX_USART1_UART_Init();
	MX_SPI1_Init();
	/* USER CODE BEGIN 2 */

	Continuous_Measurement_Mode_Value = Set_CMM_Register(Read_X_Axis_On, Read_Y_Axis_On, Read_Z_Axis_On, DRDM, Continuous_Mode_On);

	RM3100.Continuous_Measurement_Value=Continuous_Measurement_Mode_Value;
	RM3100.Cycle_Count_Value=CC_100;
	RM3100.TMRC=Rate_150Hz;

	RM3100_Init_Status = RM3100_Init(RM3100.Continuous_Measurement_Value, RM3100.TMRC, RM3100.Cycle_Count_Value);

	//Hard Iron Calibration Settings
	//Calculate from MotionCal Magnetic Calibration Tool
	const float hard_iron[3] = { 7.06, -5.32, 4.93 };

	//Soft Iron Calibration Settings
	//Calculate from MotionCal Magnetic Calibration Tool
	const float soft_iron[3][3] = { { 0.974, 0.016, 0.026 }, { 0.016, 0.988, -0.009 }, { 0.026, -0.009, 1.040 } };

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		static float hi_cal[3];
		float mag_data[] = { Mx_uT, My_uT, Mz_uT }; //Calibated magnetometer value

		RM3100_status = Check_RM3100_Status();
		if (Check_RM3100_Status() == 1)
		{
			HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, RESET);
			HAL_SPI_Transmit(&hspi1, &Read_Measurement_Start_Address, 1, 100);
			HAL_SPI_Receive(&hspi1, buffer, 9, 100);
			HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, SET);
		}

		Mx = Convert_Measurement_to_Int24(buffer[0], buffer[1], buffer[2]);
		My = Convert_Measurement_to_Int24(buffer[3], buffer[4], buffer[5]);
		Mz = Convert_Measurement_to_Int24(buffer[6], buffer[7], buffer[8]);

		float gain = (0.3671 * (float)RM3100.Cycle_Count_Value) + 1.5;


		Mx_uT=Mx/gain;
		My_uT=My/gain;
		Mz_uT=Mz/gain;


		//sprintf(sendData, "Raw:0,0,0,0,0,0,%d,%d,%d\r\n", Mx, My, Mz);
		sprintf(sendData, "%.2f,%.2f,%.2f\r\n", Mx_uT, My_uT, Mz_uT);
		HAL_UART_Transmit(&huart1, (uint8_t*) sendData, strlen(sendData), 1000);


		//Apply Hard iron offsets
		for (int i = 0; i < 3; i++)
		{
			hi_cal[i] = mag_data[i] - hard_iron[i];
		}

	  //Apply soft iron scaling
		for (int i = 0; i < 3; i++)
		{
			mag_data[i] = (soft_iron[i][0] * hi_cal[0]) + (soft_iron[i][1] * hi_cal[1]) + (soft_iron[i][2] * hi_cal[2]);
		}

		heading_cal = atan2(mag_data[1], mag_data[0]);
		heading_cal = (heading_cal * 180 / M_PI) + magnetic_declination;

		if (heading_cal < 0)
		{
			heading_cal += 2 * M_PI;
		}

		HAL_Delay(10);


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
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 84;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(RM3100_CS_GPIO_Port, RM3100_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : RM3100_CS_Pin */
	GPIO_InitStruct.Pin = RM3100_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(RM3100_CS_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
