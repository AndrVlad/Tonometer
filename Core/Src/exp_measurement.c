#include "exp_measurement.h"

#include <stdbool.h>
#include <stdint.h>



bool measurement_works = false;
uint32_t measurement_time = 0;

uint32_t get_measurement_time() {
	return measurement_time;
}

void reset_meas_time() {

	measurement_time = 0;
	__HAL_TIM_SET_COUNTER(&htim1,0);

	return;
}

void start_measurement() {

	if (measurement_works) {
		reset_meas_time();
		measurement_works = true;
		return;
	}

	TM_start();

	//HAL_TIM_Base_Start_IT(&htim3); //start timer for ADC
	//HAL_ADC_PollForConversion(&hadc1, 100);

	// Запуск таймера времени измерения
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);

	measurement_works = true;

	return;
}

void stop_measurement() {

	TM_stop();

	measurement_works = false;
	HAL_TIM_Base_Stop(&htim1);
	//HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_Stop(&htim2);
	//__HAL_TIM_SET_COUNTER(&htim3,0);
	reset_meas_time();

	return;
}
