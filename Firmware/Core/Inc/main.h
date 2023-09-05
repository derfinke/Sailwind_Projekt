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
extern SPI_HandleTypeDef hspi4;
extern UART_HandleTypeDef huart3;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI4_CS_Pin GPIO_PIN_3
#define SPI4_CS_GPIO_Port GPIOE
#define Switch_Betriebsmodus_Pin GPIO_PIN_4
#define Switch_Betriebsmodus_GPIO_Port GPIOF
#define Windrichtungmessung_Pin GPIO_PIN_7
#define Windrichtungmessung_GPIO_Port GPIOF
#define Windgeschwindigkeitmessung_Pin GPIO_PIN_9
#define Windgeschwindigkeitmessung_GPIO_Port GPIOF
#define Strommessung_Pin GPIO_PIN_10
#define Strommessung_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define Kalibrierung_Pin GPIO_PIN_2
#define Kalibrierung_GPIO_Port GPIOC
#define Abstandsmessung_Pin GPIO_PIN_0
#define Abstandsmessung_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define Windsensor_Rx_Pin GPIO_PIN_3
#define Windsensor_Rx_GPIO_Port GPIOA
#define Drehzahl_DAC_OUT_Pin GPIO_PIN_4
#define Drehzahl_DAC_OUT_GPIO_Port GPIOA
#define Analog_IN_Optional_Pin GPIO_PIN_6
#define Analog_IN_Optional_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define Kraftmessung_Pin GPIO_PIN_0
#define Kraftmessung_GPIO_Port GPIOB
#define Button_Zurueck_Pin GPIO_PIN_1
#define Button_Zurueck_GPIO_Port GPIOB
#define LED_Automatik_Pin GPIO_PIN_2
#define LED_Automatik_GPIO_Port GPIOB
#define Betriebsmodus_2_Nur_Prototyp_Pin GPIO_PIN_12
#define Betriebsmodus_2_Nur_Prototyp_GPIO_Port GPIOF
#define Betriebsmodus_Prototyp_Pin GPIO_PIN_15
#define Betriebsmodus_Prototyp_GPIO_Port GPIOF
#define Ext_Relais_1_Pin GPIO_PIN_15
#define Ext_Relais_1_GPIO_Port GPIOE
#define Ext_Relais_2_Pin GPIO_PIN_10
#define Ext_Relais_2_GPIO_Port GPIOB
#define LED_Trimmen_Pin GPIO_PIN_11
#define LED_Trimmen_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define Endschalter_Hinten_Pin GPIO_PIN_15
#define Endschalter_Hinten_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define LED_Kalibrieren_Speichern_Pin GPIO_PIN_11
#define LED_Kalibrieren_Speichern_GPIO_Port GPIOD
#define LED_Stoerung_Pin GPIO_PIN_12
#define LED_Stoerung_GPIO_Port GPIOD
#define LED_Manuell_Pin GPIO_PIN_13
#define LED_Manuell_GPIO_Port GPIOD
#define IN_1_Pin GPIO_PIN_6
#define IN_1_GPIO_Port GPIOC
#define OUT_1_Pin GPIO_PIN_9
#define OUT_1_GPIO_Port GPIOC
#define OUT_2_Pin GPIO_PIN_10
#define OUT_2_GPIO_Port GPIOC
#define OUT_3_Pin GPIO_PIN_11
#define OUT_3_GPIO_Port GPIOC
#define IN_3_Pin GPIO_PIN_12
#define IN_3_GPIO_Port GPIOC
#define IN_2_Pin GPIO_PIN_2
#define IN_2_GPIO_Port GPIOD
#define HOLD_Pin GPIO_PIN_3
#define HOLD_GPIO_Port GPIOD
#define Windsensor_Tx_Pin GPIO_PIN_5
#define Windsensor_Tx_GPIO_Port GPIOD
#define Windsensor_EN_Pin GPIO_PIN_6
#define Windsensor_EN_GPIO_Port GPIOD
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define Button_Vorfahren_Pin GPIO_PIN_4
#define Button_Vorfahren_GPIO_Port GPIOB
#define LED_PWR_Pin GPIO_PIN_6
#define LED_PWR_GPIO_Port GPIOB
#define Endschalter_Vorne_Pin GPIO_PIN_8
#define Endschalter_Vorne_GPIO_Port GPIOB
#define IN_0_Pin GPIO_PIN_9
#define IN_0_GPIO_Port GPIOB
#define LED_Rollen_Pin GPIO_PIN_0
#define LED_Rollen_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
