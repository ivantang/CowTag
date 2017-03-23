/*
 * Sleep.c
 *
 *  Created on: Mar 4, 2017
 *      Author: champ
 */

#include <Sleep.h>
#include <ti/sysbios/knl/Clock.h>
#include "global_cfg.h"
#include "bootTimestamp.h"

long long int hourSleepTicks = HOUR_SLEEP_TICKS;
long long int minuteSleepTicks = MINUTE_SLEEP_TICKS;

long int sleepAnHour() {
	sleep_offset += 3600;
	return hourSleepTicks / Clock_tickPeriod;
}

long int sleepAMinute() {
	sleep_offset += 60;
	return minuteSleepTicks / Clock_tickPeriod;
}

long int sleepFiveSeconds() {
	sleep_offset += 5;
	return minuteSleepTicks * 5 / 60 / Clock_tickPeriod;
}

long int sleepASecond() {
	sleep_offset += 1;
	return minuteSleepTicks / 60 / Clock_tickPeriod;
}

