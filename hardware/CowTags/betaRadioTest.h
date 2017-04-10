/*
 * betaRadioTest.h
 *
 * This is a test usage of the Beta tag configuration.
 * Betas use RadioSend.c/.h to to continually send their
 * test data to any nearby Alphas or a Gateway.
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef TASKS_BETATASK_H_
#define TASKS_BETATASK_H_

/***** Prototypes *****/
void betaRadioTest_init(void);
void file_printSampleData(struct sampleData sampledata);

#endif /* TASKS_BETATASK_H_ */
