/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
static void UART_TransmitBoxBorder(void);
static void UART_TransmitBoxLine(const char *message, uint16_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define UART_BOX_CONTENT_WIDTH 62U

static void UART_TransmitBoxBorder(void)
{
    char border[UART_BOX_CONTENT_WIDTH + 7U];

    memset(border, '-', UART_BOX_CONTENT_WIDTH + 4U);
    border[UART_BOX_CONTENT_WIDTH + 4U] = '\r';
    border[UART_BOX_CONTENT_WIDTH + 5U] = '\n';

    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)border,
        UART_BOX_CONTENT_WIDTH + 6U,
        HAL_MAX_DELAY
    );
}

static void UART_TransmitBoxLine(const char *message, uint16_t length)
{
    char line[UART_BOX_CONTENT_WIDTH + 7U];
    uint16_t printableLength = length;
    uint16_t offset = 0;

    while (printableLength > 0U &&
           (message[printableLength - 1U] == '\r' ||
            message[printableLength - 1U] == '\n'))
    {
        printableLength--;
    }

    do
    {
        uint16_t remaining = printableLength - offset;
        uint16_t chunkLength = remaining > UART_BOX_CONTENT_WIDTH
            ? UART_BOX_CONTENT_WIDTH
            : remaining;

        line[0] = '|';
        line[1] = ' ';
        memset(&line[2], ' ', UART_BOX_CONTENT_WIDTH);
        memcpy(&line[2], &message[offset], chunkLength);
        line[UART_BOX_CONTENT_WIDTH + 2U] = ' ';
        line[UART_BOX_CONTENT_WIDTH + 3U] = '|';
        line[UART_BOX_CONTENT_WIDTH + 4U] = '\r';
        line[UART_BOX_CONTENT_WIDTH + 5U] = '\n';

        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)line,
            UART_BOX_CONTENT_WIDTH + 6U,
            HAL_MAX_DELAY
        );

        offset += chunkLength;
    } while (offset < printableLength);
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
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  char msg[64];

  /* ADC calibration - unrelated to I2C, okay to leave here */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Give AHT20 time after power-up */
  HAL_Delay(100);

  UART_TransmitBoxBorder();

  /* ============================= */
  /* AHT20 INITIALIZATION          */
  /* ============================= */

  uint8_t initCmd[3] = {0xBE, 0x08, 0x00};

  HAL_StatusTypeDef ahtInitStatus = HAL_I2C_Master_Transmit(
      &hi2c1,
      0x38 << 1,
      initCmd,
      3,
      100
  );

  if (ahtInitStatus == HAL_OK)
  {
    char ahtMsg[] = "AHT20 init OK\r\n";

    UART_TransmitBoxLine(ahtMsg, sizeof(ahtMsg) - 1);
  }
  else
  {
    int len = snprintf(
        msg,
        sizeof(msg),
        "AHT20 init failed: status=%d error=0x%08lX\r\n",
        ahtInitStatus,
        HAL_I2C_GetError(&hi2c1)
    );

    UART_TransmitBoxLine(msg, (uint16_t)len);
  }

  /* Give BH1750 time after power-up */
  HAL_Delay(40);

  /* BH1750 mode command */
  uint8_t bh1750Mode = 0x10;

  HAL_I2C_Master_Transmit(
      &hi2c1,
      0x23 << 1,
      &bh1750Mode,
      1,
      100
  );

  HAL_Delay(40);
  UART_TransmitBoxBorder();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1)
{
    UART_TransmitBoxBorder();
    int len;

    /* ==================== AHT20 ==================== */

    uint8_t triggerCmd[3] = {0xAC, 0x33, 0x00};
    uint8_t data[6];

    HAL_StatusTypeDef ahtResult;

    /* Tell AHT20 to take a measurement */
    ahtResult = HAL_I2C_Master_Transmit(
        &hi2c1,
        0x38 << 1,
        triggerCmd,
        3,
        100
    );

    if (ahtResult == HAL_OK)
    {
        HAL_Delay(80);

        /* Read measurement */
        ahtResult = HAL_I2C_Master_Receive(
            &hi2c1,
            0x38 << 1,
            data,
            6,
            100
        );

        if (ahtResult == HAL_OK)
        {
            uint32_t rawTemperature =
                ((uint32_t)(data[3] & 0x0F) << 16) |
                ((uint32_t)data[4] << 8) |
                data[5];

            float temperature =
                ((float)rawTemperature * 200.0f / 1048576.0f)
                - 50.0f;

            uint32_t rawHumidity =
                ((uint32_t)data[1] << 12) |
                ((uint32_t)data[2] << 4) |
                ((data[3] & 0xF0) >> 4);

            float humidity =
                ((float)rawHumidity * 100.0f / 1048576.0f);

            len = snprintf(
                msg,
                sizeof(msg),
                "Temperature: %.2f C | Humidity: %.2f %%\r\n",
                temperature,
                humidity
            );

            UART_TransmitBoxLine(msg, (uint16_t)len);
        }
        else
        {
            char error[80];
            snprintf(error, sizeof(error),
                     "AHT20 read failed: status=%u error=0x%08lX\r\n",
                     (unsigned int)ahtResult,
                     (unsigned long)HAL_I2C_GetError(&hi2c1));

            UART_TransmitBoxLine(error, (uint16_t)strlen(error));
        }
    }
    else
    {
        char error[80];
        snprintf(error, sizeof(error),
                 "AHT20 command failed: status=%u error=0x%08lX\r\n",
                 (unsigned int)ahtResult,
                 (unsigned long)HAL_I2C_GetError(&hi2c1));

        UART_TransmitBoxLine(error, (uint16_t)strlen(error));
    }

    /* ==================== BH1750 ==================== */

    uint8_t bhData[2];

    HAL_StatusTypeDef bhResult =
        HAL_I2C_Master_Receive(
            &hi2c1,
            0x23 << 1,
            bhData,
            2,
            100
        );

    if (bhResult == HAL_OK)
    {
        uint16_t rawLight =
            ((uint16_t)bhData[0] << 8) |
            bhData[1];

        float lux = rawLight / 1.2f;

        len = snprintf(
            msg,
            sizeof(msg),
            "BH1750 | Raw: %u | Light: %.2f lux\r\n",
            rawLight,
            lux
        );

        UART_TransmitBoxLine(msg, (uint16_t)len);
    }
    else
    {
        char error[80];
        snprintf(error, sizeof(error),
                 "BH1750 read failed: status=%u error=0x%08lX\r\n",
                 (unsigned int)bhResult,
                 (unsigned long)HAL_I2C_GetError(&hi2c1));

        UART_TransmitBoxLine(error, (uint16_t)strlen(error));
    }


    /* ==================== SOIL SENSOR ==================== */

    HAL_StatusTypeDef adcStart =
        HAL_ADC_Start(&hadc1);

    if (adcStart == HAL_OK)
    {
        HAL_StatusTypeDef adcPoll =
            HAL_ADC_PollForConversion(&hadc1, 100);

        if (adcPoll == HAL_OK)
        {
            uint32_t soilRaw =
                HAL_ADC_GetValue(&hadc1);

            len = snprintf(
                msg,
                sizeof(msg),
                "Soil raw ADC: %lu\r\n",
                (unsigned long)soilRaw
            );

            UART_TransmitBoxLine(msg, (uint16_t)len);
        }
        else
        {
            char error[] = "Soil ADC conversion failed\r\n";

            UART_TransmitBoxLine(error, sizeof(error) - 1);
        }
    }
    else
    {
        char error[] = "Soil ADC start failed\r\n";

        UART_TransmitBoxLine(error, sizeof(error) - 1);
    }
    HAL_ADC_Stop(&hadc1);

    UART_TransmitBoxBorder();


    /* Wait before next sensor update */
    HAL_Delay(2000);
  }
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
#ifdef USE_FULL_ASSERT
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
