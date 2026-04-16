/*
 * sensor_utils.c
 *
 *  Created on: 20 мар. 2026 г.
 *      Author: AVA
 */

#include "sensor_utils.h"
#include "main.h"
#include "w25q_spi.h"
#include "protocol_parser.h"
#include "SPI_Connection.h"
//#include "stm32f1xx_hal.h"
//#include "tonometer_logic.h"
#include <stdbool.h>
#include <stdint.h>

bool need_selfcheck = 0;

/* Выполняет проверку работоспособности датчика
 * Возврат: 1 - датчик работоспособен
 * 			0 - датчик не работоспособен */
uint16_t val1, val2;
bool sensorSelfCheck() {
#ifdef TEST_VERSION
	return true;
#endif



	HAL_ADC_Start(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, 300) == HAL_OK) {
        val1 = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    if (val1 == 0 || val1 == 4095) {
        return false;
    }



    HAL_ADC_Start(&hadc2);

    if (HAL_ADC_PollForConversion(&hadc2, 300) == HAL_OK) {
        val2 = HAL_ADC_GetValue(&hadc2);
    }
    HAL_ADC_Stop(&hadc2);

    if (val2 == 0 || val2 == 4095) {
        return false;
    }

	/*
	//HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2,100);
	val = HAL_ADC_GetValue(&hadc2);
	HAL_ADC_Stop(&hadc2);
	if (val < 100 || val > 65000)
		return false; */


	return true;
}

void sensorChipInit() {
	HAL_ADC_Start(&hadc1);
	HAL_Delay(20);
	HAL_ADC_PollForConversion(&hadc1, 300);

    HAL_ADC_Start(&hadc2);
    HAL_Delay(20);
    HAL_ADC_PollForConversion(&hadc2, 300);

	TM_init();

	return;
}

void resetSensorChip() {
	TM_init();

	return;
}

void stopSensorChip() {
	return;
}

void enableSensorChip() {
	return;
}

/* Выполняет сброс датчика */
void resetSensor() {

	stopMeasurement();

    // сброс cостояния протокола
    resetFSMProtocol();

    // сброс переменных
    page_pos_ptr = 0;
    page_ptr = 0;

    // сброс микросхемы датчика
    resetSensorChip();

	// очистка флеш-памяти
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
    W25_Erase_Chip();

    reset_ready = true;

	return;
}
/* Запуск измерения */
void startMeasurement() {
	TM_start();

	enableSensorChip();
	return;
}

/* Остановка измерения */
void stopMeasurement() {
	TM_stop();

	stopSensorChip();
	return;
}



