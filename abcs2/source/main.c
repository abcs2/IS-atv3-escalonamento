#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME 10


typedef struct task {
	char name[MAX_NAME];
	int period;
	int deadline;
	int burst;
	
	int timeFromStart;
	int timeFromExec;
	
	int lostDeadlinesCount;
	int completeExecutionCount;
	int killedCount;
	
	struct task *next;
} Task;

typedef struct {
    Task *head;
} TaskList;

typedef struct {
    Task *head;
    Task *tail;
} TaskQueue;



void addTaskToList(TaskList *taskList, Task *task) {
    return;
}

void addTaskToQueue(TaskQueue *taskQueue, Task *task) {
    return;
}

Task *getTaskFromQueue(TaskQueue *taskQueue, Task *task) {
    return task;
}

void updateTaskQueue(TaskQueue *taskQueue) {
    return;
}

Task *getMostPriority(TaskQueue *taskQueue, int maxTicks) {
    Task *priority = NULL, *aux = taskQueue->head;
    int period = maxTicks;

    while (aux != NULL) {
        // rate monotonic
        if (aux->period < period) {
            priority = aux;
            period = aux->period;
        }
        aux = aux->next;
    }

    return priority;
}

int waitForTask(TaskQueue *taskQueue, int *totalTicks, int maxTicks) {
    int currentCounter = 0, code;
    while (1) {
        if ((*totalTicks) == maxTicks) {
            // kill remaining tasks
            code = 0;
            break;
        }

        if (taskQueue->head != NULL) {
            code = 1;
            break;
        }

        (*totalTicks)++;
        currentCounter++;
        updateTaskQueue(taskQueue);
    }
    if (currentCounter > 0) {
        printf("idle for %d units", currentCounter);
    }
    return code;
}

void randomBoringFunctionThatExistsToCallWaitForTaskBoringFunction(TaskQueue *taskQueue, int *totalTicks, int maxTicks) {
    int code = waitForTask(taskQueue, totalTicks, maxTicks);

    if (code == 0) {
        // kill remaining tasks
    } else if (code == 1) {
        runTask(taskQueue, getMostPriority(taskQueue, maxTicks), totalTicks, maxTicks);
    }
}

void updateTimeFromStart(TaskQueue *taskQueue) {
    Task *aux = taskQueue->head;
    while (aux != NULL) {
        aux->timeFromStart++;
        aux = aux->next;
    }
    return;
}

char getStatus(Task *task, int *totalTicks, int maxTicks) {
    char status = ' ';
    if (task->timeFromExec == task->burst) {
		status = 'F';
        task->completeExecutionCount++;
	} else if (task->timeFromStart == task->deadline) {
		status = 'L';
        task->lostDeadlinesCount++;
	} else if ((*totalTicks) == maxTicks) {
		status = 'K';
        task->killedCount++;
	} else {
		status = 'H';
	}
    return status;
}

void runTask(TaskQueue *taskQueue, Task *task, int *totalTicks, int maxTicks) {
	int currentCounter = 0;
	char status = ' ';
	while ((task->timeFromStart < task->deadline) && (task->timeFromExec < task->burst) && ((*totalTicks) < maxTicks)) {
		// execute task...
		(*totalTicks)++;
		updateTimeFromStart(taskQueue);
		task->timeFromExec++;
		currentCounter++;

        updateTaskQueue(taskQueue);
        if (task != getMostPriority(taskQueue, maxTicks)) {
            break;
        } 
	}
    // execution interrupted or finished
    status = getStatus(task, totalTicks, maxTicks);

    printf("[%s] for %d units - %c\n", task->name, currentCounter, status);
    return;
}

int main(int argc, char **argv) {

    return 0;
}