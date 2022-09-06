 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port_Cfg.h
 *
 * Description: Pre-Compile Configuration Header file for TM4C123GH6PM Microcontroller - Port Driver
 *
 * Author: Elsayed Ayman Elsayed Ali Habib
 ******************************************************************************/

#ifndef PORT_CFG_H
#define PORT_CFG_H

/*
 * Module Version 1.0.0
 */
#define PORT_CFG_SW_MAJOR_VERSION              (1U)
#define PORT_CFG_SW_MINOR_VERSION              (0U)
#define PORT_CFG_SW_PATCH_VERSION              (0U)

/*
 * AUTOSAR Version 4.0.3
 */
#define PORT_CFG_AR_RELEASE_MAJOR_VERSION     (4U)
#define PORT_CFG_AR_RELEASE_MINOR_VERSION     (0U)
#define PORT_CFG_AR_RELEASE_PATCH_VERSION     (3U)

/* Pre-compile option for Development Error Detect */
#define PORT_DEV_ERROR_DETECT                (STD_ON)

/* Pre-compile option for Version Info API */
#define PORT_VERSION_INFO_API                (STD_ON)

/* Pre-compile option for SET_PIN_DIRECTION Info API */
#define PORT_SET_PIN_DIRECTION_API           (STD_ON)

/* Pre-compile option for SET_PIN_MODE Info API */
#define PORT_SET_PIN_MODE_API                (STD_ON)

/* Number of the configured Port Channels */
#define PORT_CONFIGURED_CHANNLES              (43U)

/*ha3mel hena arkam lel modes bel arkam eli kanet mawgoda fel gadwal*/
#define  PORT_PIN_MODE_ADC   (0U)
#define  PORT_PIN_MODE_UART  (1U)
#define  PORT_PIN_MODE_SSI   (2U)
#define  PORT_PIN_MODE_UART1 (2U)
#define  PORT_PIN_MODE_I2C   (3U)
#define  PORT_PIN_MODE_CAN0  (3U)
#define  PORT_PIN_MODE_PWM0  (4U)
#define  PORT_PIN_MODE_PWM1  (5U)
#define  PORT_PIN_MODE_QEI   (6U)
#define  PORT_PIN_MODE_GPT   (7U)
#define  PORT_PIN_MODE_CAN   (8U)
#define  PORT_PIN_MODE_USB   (8U)
#define  PORT_PIN_NOT_ACTIVE (9U)
#define  PORT_PIN_MODE_DIO   (10U)

#define PORT_PortA                    (0U)
#define PORT_PortB                    (1U)
#define PORT_PortC                    (2U)
#define PORT_PortD                    (3U)
#define PORT_PortE                    (4U)
#define PORT_PortF                    (5U)


#define PORT_Pin0                     (0U)
#define PORT_Pin1                     (1U)
#define PORT_Pin2                     (2U)
#define PORT_Pin3                     (3U)
#define PORT_Pin4                     (4U)
#define PORT_Pin5                     (5U)
#define PORT_Pin6                     (6U)
#define PORT_Pin7                     (7U)

#define PORT_PIN_LEVEL_LOW STD_LOW
#define PORT_PIN_LEVEL_HIGH STD_HIGH


#endif /* PORT_CFG_H */
