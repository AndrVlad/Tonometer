/*
 * exp_measurement.h
 *
 *  Created on: Jul 21, 2026
 *      Author: vlado
 */

#ifndef INC_EXP_MEASUREMENT_H_
#define INC_EXP_MEASUREMENT_H_

#define led_on() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
#define led_off() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
#define led_switch() HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);

#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim4;

uint32_t get_measurement_time();
void start_measurement();
void stop_measurement();

#endif /* INC_EXP_MEASUREMENT_H_ */
