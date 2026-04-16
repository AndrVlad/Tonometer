#ifndef TONOMETER_LOGIC_H
#define TONOMETER_LOGIC_H

#include "main.h"
#include "tonometer_maths.h"
#include <stdbool.h>

// GLOBALS

#define TMS_NONE 		0
#define TMS_PUMPING 	1
#define TMS_DEFLATING 	2

uint8_t TMS = TMS_NONE;

#define ADC_MM_MULTIPLIER 0.1675
#define MM_ADC_MULTIPLIER (1.0 / ADC_MM_MULTIPLIER)
#define PRESSURE_MAX (200 - 30)
#define PRESSURE_MIN 50
#define PRESSURE_MAX_CODE (PRESSURE_MAX * MM_ADC_MULTIPLIER)
#define PRESSURE_MIN_CODE (PRESSURE_MIN * MM_ADC_MULTIPLIER)

#define GPIO_PIN_PUMP 		GPIO_PIN_15
#define GPIO_PIN_SOLENOID	GPIO_PIN_14

//extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

uint16_t lastMainPressure = 0;
uint16_t pulsePressure = 0;

bool gotResult = false;

int8_t newValue = 0;

uint16_t timerCnt = 0;


// FUNCTIONS
void beginPumping();
void beginDeflating();
void beginFinishing();

bool hasData() {
	return gotResult;
}

void TM_init() {
	gotResult = false;
	TMS = TMS_NONE;

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_PUMP, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_SOLENOID, GPIO_PIN_RESET);

	HAL_TIM_Base_Stop_IT(&htim4);
	HAL_TIM_Base_Stop_IT(&htim3);
}

void TM_start() {
	if (TMS == TMS_NONE) {
		gotResult = false;
		initM();
		beginPumping();
	}
}

void TM_stop() {
	if (TMS != TMS_NONE)
		beginFinishing();
}

void TM_Cycle() {
	if (TMS == TMS_NONE)
		return;
	if (newValue == 1) {
		switch(TMS) {
		case TMS_PUMPING:
			if (lastMainPressure > PRESSURE_MAX_CODE)
				beginDeflating();
			break;

		case TMS_DEFLATING:
			setData(pulsePressure, lastMainPressure);
			if (lastMainPressure < PRESSURE_MIN_CODE) {
				gotResult = true;
				beginFinishing();
			}
			break;
		}
		if (TMS == TMS_DEFLATING) {
			HAL_UART_Transmit(&huart1, (uint8_t*)&lastMainPressure, 2, 100);
			HAL_UART_Transmit(&huart1, (uint8_t*)&pulsePressure, 2, 100);
		}
	}
	newValue = 0;
}

void beginPumping() {
	// Начинает накачку воздуха, запускает сторожевой таймер на полминуты
	TMS = TMS_PUMPING;

//	htim4.Init.Period = 45000;
//	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
//	{
//		Error_Handler();
//	}

	timerCnt = 0;
	HAL_TIM_Base_Start_IT(&htim4);


	HAL_TIM_Base_Start_IT(&htim3);

	HAL_ADC_Start_IT(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	HAL_ADC_Start_IT(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, 100);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_PUMP, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_SOLENOID, GPIO_PIN_SET);
}

void beginDeflating() {
	// Прекращает накачку воздуха, запускает сторожевой таймер на минуту
	TMS = TMS_DEFLATING;

	HAL_TIM_Base_Stop_IT(&htim4);

//	htim4.Init.Period = 65000;
//	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
//	{
//		Error_Handler();
//	}

	timerCnt = 0;
	HAL_TIM_Base_Start_IT(&htim4);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_PUMP, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_SOLENOID, GPIO_PIN_SET);
}

int32_t APNormalized;
void beginFinishing() {

	// Начинает спуск воздуха
	HAL_TIM_Base_Stop_IT(&htim4);
	HAL_TIM_Base_Stop_IT(&htim3);
	timerCnt = 0;


	if (TMS == TMS_DEFLATING && gotResult) {
		finalize();
		HAL_UART_Transmit(&huart1, (uint8_t*)&currentAP, 4, 100);
		APNormalized = 0;
		APNormalized |= (currentAP.systolic);
		APNormalized |= (currentAP.diastolic << 8);
		writeDataToFlash(APNormalized);
	}

	TMS = TMS_NONE;


	gotResult = false;

	//HAL_ADC_Stop_IT(&hadc2);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_PUMP, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_SOLENOID, GPIO_PIN_RESET);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (TMS == TMS_NONE)
		return;
	if (hadc->Instance == ADC1) {
		lastMainPressure = HAL_ADC_GetValue(hadc);
	}
	else if (hadc->Instance == ADC2) {
		pulsePressure = HAL_ADC_GetValue(hadc);

		newValue = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim -> Instance == TIM3) {
//		if (TMS == TMS_PUMPING)
//			beginDeflating();
//		if (TMS == TMS_DEFLATING)
//			beginFinishing();
	}
	if (htim->Instance == TIM4) {
		timerCnt ++;
		if (TMS == TMS_PUMPING && timerCnt == 45)
			beginDeflating();
		if (TMS == TMS_DEFLATING && timerCnt == 90)
			beginFinishing();
//		HAL_ADC_Start_IT(&hadc1);
//		HAL_ADC_Start_IT(&hadc2);
	}
}

#endif
