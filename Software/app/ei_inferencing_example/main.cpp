/** Copyright © 2021 The Things Industries B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file main.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "classifier/ei_run_classifier.h"

UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void Error_Handler(void);
static void MX_USART2_UART_Init(void);

void ei_printf(const char *format, ...);
void vprint(const char *fmt, va_list argp);
void run_inference();

float features[] = {
    // copy and replace raw features here (for example from the 'Live classification' page)
    -36, -490, 76, -45, -498, 84, -36, -490, 77, -48, -502, 85, -40, -505, 86, -23, -524, 89, 2, -589, 71, -36, -531, 56, -45, -524, 61, -158, -468, 72, -229, -414, 82, -296, -360, 64, -298, -350, 60, -360, -280, 56, -434, -198, 61, -457, -203, 72, -465, -195, 63, -438, -216, 84, -368, -342, 77, -299, -374, 45, -292, -383, 41, -254, -401, 37, -206, -408, 29, -144, -460, 53, -137, -467, 51, -101, -491, 76};

/**
 * --------------------------- GENERAL FUNCTIONS ----------------------------------
 */

/**
 * @brief System Clock Configuration
 * @return None
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @return None
 */
static void Error_Handler(void)
{
    while (1)
    {
    }
}

/*
 * --------------------------- Edge Impulse functions ---------------------------
 */

/**
 * Low-level print function that uses UART to print status messages.
 */
void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if (0 < vsprintf(string, fmt, argp)) // build string
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)string, strlen(string), 0xffffff);
    }
}

/**
 * Wrapper for vprint. Use this like you would printf to print messages to the serial console.
 */
void ei_printf(const char *format, ...)
{
    va_list myargs;
    va_start(myargs, format);
    vprint(format, myargs);
    va_end(myargs);
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr)
{
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

/**
 * @brief      Run inference
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */

void run_inference()
{
    ei_printf("Edge Impulse standalone inferencing\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
    {
        ei_printf(
            "The size of your 'features' array is not correct. Expected %d items, but had %u\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
            sizeof(features) / sizeof(float));
        return;
    }

    ei_impulse_result_t result = {0};

    // the features are stored into flash, and we don't want to load everything into RAM
    ei_printf("Getting signal...");
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;
    ei_printf("Done\n");

    ei_printf("Run classifier...");
    // Feed signal to the classifier
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    ei_printf("Done\n");

    // Returned error variable "res" while data object.array in "result"
    ei_printf("run_classifier returned: %d\n", res);
    if (res != 0)
        return;

    // print the predictions
    ei_printf(
        "Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
        result.timing.dsp, result.timing.classification,
        result.timing.anomaly);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
        int val = (int)((result.classification[ix].value * 100) + 0.5);
        ei_printf("    %s: \t %i%%\r\n", result.classification[ix].label, val);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %f\r\n", result.anomaly);
#endif
}

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();

    //	GNSE_BSP_LED_Init(LED_RED);
    //	GNSE_BSP_LED_Init(LED_GREEN);
    //	GNSE_BSP_LED_Init(LED_BLUE);

    // Say some stuff
    ei_printf("Inferencing settings:\r\n");
    ei_printf("\tInterval: %d ms.\r\n", EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\r\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\r\n",
              EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\r\n\n",
              sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    run_inference();

    while (1)
    {
        //		GNSE_BSP_LED_Toggle(LED_RED);
        //		HAL_Delay(100);
        //		GNSE_BSP_LED_Toggle(LED_GREEN);
        //		HAL_Delay(100);
        //		GNSE_BSP_LED_Toggle(LED_BLUE);
        //		HAL_Delay(100);
    }
    return 0;
}
