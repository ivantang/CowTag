/*
 * TaskManager.c
 *
 *  Created on: Mar 5, 2017
 *      Author: champ
 */

#include <TaskManager.h>


Task_Handle *taskList[TOTAL_TASKS];

void addTaskHandle(int taskID, Task_Handle *taskHandle)
{
	taskList[taskID] = taskHandle;
}

Task_Handle getTaskHandle(int taskID)
{
	return taskList[taskID];
}
