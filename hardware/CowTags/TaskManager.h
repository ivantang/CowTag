/*
 * TaskManager.h
 *
 *  Created on: Mar 5, 2017
 *      Author: champ
 */

#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_

#include <ti/sysbios/knl/Task.h>

/* Task ID's */
#define TOTAL_TASKS	       4
#define TASK_BETA          0
#define TASK_ALPHA         1
#define TASK_RADIO_SEND    2
#define TASK_RADIO_RECEIVE 3

/* Task Priorities */
#define TASK_DEFAULT_PRI  268552701
#define TASK_INACTIVE_PRI 0

void addTaskHandle(int taskID, Task_Handle *taskHandle);
Task_Handle getTaskHandle(int taskID);

#endif /* TASKMANAGER_H_ */
