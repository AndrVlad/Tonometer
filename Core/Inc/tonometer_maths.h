/*
 * tonometer_maths.h
 *
 *  Created on: Mar 22, 2026
 *      Author: StSrg
 */

#ifndef INC_TONOMETER_MATHS_H_
#define INC_TONOMETER_MATHS_H_

#include <stdint.h>
#include <stdlib.h>




int16_t peaksM [200];
int16_t pressuresM [200];
int16_t absoluteCounter = 0;
int16_t valuesCounter = 0;
int16_t risingSequenceEnd = 0;
int16_t prevValue = -1;


int16_t tmp;
#define MEDIAN_SIZE 6
int16_t medianBuffer[MEDIAN_SIZE];
int16_t countersBuffer[MEDIAN_SIZE];
int16_t pressuresBuffer[MEDIAN_SIZE];
int8_t medianCounter = 0;
int8_t rising = 0;
int8_t bigRise = 0;

int8_t j, k;
int8_t jPrev = 255;
int8_t uppers = 0, equals = 0;
int16_t prevPeak = -1;
int8_t overCtr = 0;


void initM()
{
	absoluteCounter = 0;
	valuesCounter = 0;
	prevValue = -1;
	risingSequenceEnd = 0;
	medianCounter = 0;
	rising = 0;
	bigRise = 0;
	prevPeak = -1;
	overCtr = 0;
	jPrev = 255;
	uppers = 0, equals = 0;
}


int16_t ADCToPressure(int16_t value)
{
	return value * 0.1675;
}

int16_t setData(int16_t value, int16_t pressure)
{
	if (value >= 730)
		value -= 730;
	else
		value = 0;

	pressure -= 31;




    absoluteCounter++;
    //Console.WriteLine(peaksM[1]);

    if (prevValue < value)
    {
        //Console.WriteLine(ADCToPressure(value));
        bigRise |= ((value - prevValue) >= 10 / 0.1675 ? 1 : 0);
        if (bigRise == 1)
            ;
        rising++;
    }
    if (absoluteCounter > 100 && rising >= 3 && prevValue > value && bigRise == 1)
    {
        //if (prevValue < 50 / 0.1675)
        //    ;

        rising = 0;
        bigRise = 0;

        if (medianCounter < MEDIAN_SIZE)
            medianCounter++;


        for (j = 0; j < MEDIAN_SIZE - 1; ++j)
            medianBuffer[j] = medianBuffer[j+1];
        medianBuffer[MEDIAN_SIZE-1] = prevValue;

        for (j = 0; j < MEDIAN_SIZE - 1; ++j)
            pressuresBuffer[j] = pressuresBuffer[j + 1];
        pressuresBuffer[MEDIAN_SIZE - 1] = pressure;

        for (j = 0; j < MEDIAN_SIZE - 1; ++j)
            countersBuffer[j] = countersBuffer[j + 1];
        countersBuffer[MEDIAN_SIZE - 1] = absoluteCounter;

        if (medianCounter < MEDIAN_SIZE)
            return 0;

        for (j = 0; j != MEDIAN_SIZE; ++j)
        {
            uppers = 0;
            equals = 0;
            for (k = 0; k != MEDIAN_SIZE; ++k)
                if (medianBuffer[j] < medianBuffer[k])
                    ++uppers;
                else if (medianBuffer[j] == medianBuffer[k])
                    ++equals;
            if (uppers == 2 || uppers == 1 && equals > 1)
                break;
        }
        //Console.WriteLine(j);

        prevValue = medianBuffer[j];

        if (peaksM[risingSequenceEnd] > 50 && overCtr < 3 && prevPeak != -1 && abs(prevValue - prevPeak) >= 300)
        {
            overCtr++;
            jPrev--;
            goto end;
        }
        //if (overCtr == 3)
        //    ;
        overCtr = 0;

        if (jPrev < MEDIAN_SIZE && jPrev != 0 && j < jPrev-1)
        {
            jPrev--;
            goto end;// valuesCounter--;
        }
        jPrev = j;


        prevPeak = prevValue;

        if (valuesCounter == 0)
        {
            peaksM[0] = prevValue;
            pressuresM[0] = pressuresBuffer[j];
            valuesCounter = 1;
            risingSequenceEnd = 0;
        }
        else
        {
            for (tmp = (valuesCounter - 1); tmp >= risingSequenceEnd; --tmp)
            {
                if (prevValue == peaksM[tmp])
                    goto end;
                if (prevValue < peaksM[tmp])
                {
                    break;
                }
            }
            if (tmp == risingSequenceEnd-1)
            {
                tmp = risingSequenceEnd++;
            }
            //Console.WriteLine(tmp);

            peaksM[tmp + 1] = prevValue;
            valuesCounter = (tmp + 2);
            pressuresM[tmp + 1] = pressuresBuffer[j];
        }
    }
    else if (prevValue > value)
    {
        bigRise = 0;
        rising = 0;
    }

    end:

    prevValue = value;

    return 0;
}



int16_t i;
int16_t tempMiddle;
int16_t maxPeak;

typedef struct AP
{
    int16_t systolic;
    int16_t diastolic;
} AP;
AP currentAP;


const double systolicMultiplier = 0.5;
const double diastolicMultiplier = 0.85;

int16_t awaitedSystolic;
int16_t awaitedDiatolic;
AP finalize()
{
	//valuesCounter -= 10;
	if (valuesCounter < 5) {
		currentAP.systolic = 0;
		currentAP.diastolic = 0;
		return currentAP;
	}

//	tempMiddle = peaksM[0] + peaksM[1] + peaksM[2];
//	for (i = 3; i < valuesCounter; ++i)
//	{
//		tempMiddle += peaksM[i];
//		peaksM[i - 2] = tempMiddle >> 2;
//		tempMiddle -= peaksM[i - 3];
//	}
	//valuesCounter -= 4;


	//for (i = 1; i < valuesCounter; ++i)
	//    if (peaksM[i] > peaksM[i-1])
	//    {

	//    }

	maxPeak = 0;
	for (i = 0; i < valuesCounter; ++i)
		if (peaksM[i] > maxPeak)
			maxPeak = peaksM[i];

	awaitedSystolic = (int16_t)(maxPeak * systolicMultiplier);
	awaitedDiatolic = (int16_t)(maxPeak * diastolicMultiplier);

	i = 0;
	while (i < valuesCounter && peaksM[i] < awaitedSystolic) ++i;
	if (i == 0) ++i;
	currentAP.systolic = (int16_t)(pressuresM[i - 1] + (double)(pressuresM[i] - pressuresM[i - 1]) * (awaitedSystolic - peaksM[i - 1]) / (peaksM[i] - peaksM[i - 1]));
	while (i < valuesCounter && peaksM[i] < maxPeak) ++i;
	while (i < valuesCounter && peaksM[i] > awaitedDiatolic) ++i;
	if (i == valuesCounter) --i;
	currentAP.diastolic = pressuresM[i - 1] + (double)(pressuresM[i] - pressuresM[i - 1]) * (awaitedDiatolic - peaksM[i - 1]) / (peaksM[i] - peaksM[i - 1]);



	currentAP.diastolic = (int16_t)(ADCToPressure(currentAP.diastolic) );
	currentAP.systolic = (int16_t)(ADCToPressure(currentAP.systolic) );

//	currentAP.diastolic = (int16_t)(currentAP.diastolic * 1.2);

	return currentAP;
}


#endif /* INC_TONOMETER_MATHS_H_ */
