/*
 * Sleep.c
 *
 *  Created on: Mar 4, 2017
 *      Author: champ
 */

#include <Sleep.h>
#include <ti/sysbios/knl/Clock.h>

long long int hourSleepTicks = 3600000000;
long long int minuteSleepTicks = 60000000;

long int sleepAnHour() {
	return hourSleepTicks / Clock_tickPeriod;
}

long int sleepAMinute() {
	return minuteSleepTicks / Clock_tickPeriod;
}

long int sleepASecond() {
	return minuteSleepTicks / 60 / Clock_tickPeriod;
}

