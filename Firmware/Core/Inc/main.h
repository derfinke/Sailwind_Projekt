/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_1_Red_PE2_Pin GPIO_PIN_2
#define LED_1_Red_PE2_GPIO_Port GPIOE
#define LED_2_Yellow_PE3_Pin GPIO_PIN_3
#define LED_2_Yellow_PE3_GPIO_Port GPIOE
#define LED_3_Green_PE4_Pin GPIO_PIN_4
#define LED_3_Green_PE4_GPIO_Port GPIOE
#define LED_4_Green_PE5_Pin GPIO_PIN_5
#define LED_4_Green_PE5_GPIO_Port GPIOE
#define Strommessung_ADC3_IN_Pin GPIO_PIN_9
#define Strommessung_ADC3_IN_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define Kraftmessung_ADC1_IN_Pin GPIO_PIN_0
#define Kraftmessung_ADC1_IN_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define Windsensor_USART2_Rx_Pin GPIO_PIN_3
#define Windsensor_USART2_Rx_GPIO_Port GPIOA
#define Drehzahl_DAC_OUT_Pin GPIO_PIN_4
#define Drehzahl_DAC_OUT_GPIO_Port GPIOA
#define Windgeschwindigkeit_ADC2_IN_Pin GPIO_PIN_5
#define Windgeschwindigkeit_ADC2_IN_GPIO_Port GPIOA
#define Windrichtung_ADC2_IN_Pin GPIO_PIN_6
#define Windrichtung_ADC2_IN_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define Abstandsmessung_ADC1_IN_Pin GPIO_PIN_1
#define Abstandsmessung_ADC1_IN_GPIO_Port GPIOB
#define OUT2_PF12_IN_Pin GPIO_PIN_12
#define OUT2_PF12_IN_GPIO_Port GPIOF
#define OUT3_PF13_IN_Pin GPIO_PIN_13
#define OUT3_PF13_IN_GPIO_Port GPIOF
#define Button_1_Links_PE10_Pin GPIO_PIN_10
#define Button_1_Links_PE10_GPIO_Port GPIOE
#define Button_2_Rechts_PE12_Pin GPIO_PIN_12
#define Button_2_Rechts_PE12_GPIO_Port GPIOE
#define Button_3_Kalibrierung_PE14_Pin GPIO_PIN_14
#define Button_3_Kalibrierung_PE14_GPIO_Port GPIOE
#define Button_4_Reserviert_PE15_Pin GPIO_PIN_15
#define Button_4_Reserviert_PE15_GPIO_Port GPIOE
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define OUT1_PD15_IN_Pin GPIO_PIN_15
#define OUT1_PD15_IN_GPIO_Port GPIOD
#define UART_EN_PD2_Pin GPIO_PIN_2
#define UART_EN_PD2_GPIO_Port GPIOD
#define IN4_PD3_OUT_Pin GPIO_PIN_3
#define IN4_PD3_OUT_GPIO_Port GPIOD
#define IN3_PD4_OUT_Pin GPIO_PIN_4
#define IN3_PD4_OUT_GPIO_Port GPIOD
#define Windsensor_USART2_Tx_Pin GPIO_PIN_5
#define Windsensor_USART2_Tx_GPIO_Port GPIOD
#define IN1_PD6_OUT_Pin GPIO_PIN_6
#define IN1_PD6_OUT_GPIO_Port GPIOD
#define IN0_PD7_OUT_Pin GPIO_PIN_7
#define IN0_PD7_OUT_GPIO_Port GPIOD
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define Endschalter_Vorne_PB3_Pin GPIO_PIN_3
#define Endschalter_Vorne_PB3_GPIO_Port GPIOB
#define Endschalter_Hinten_PB4_Pin GPIO_PIN_4
#define Endschalter_Hinten_PB4_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
