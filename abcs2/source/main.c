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
	
	struct task *nextList;
    struct task *nextQueue;
} Task;

typedef struct {
    Task *head;
} TaskList;

typedef struct {
    Task *head;
} TaskQueue;



void freeTasks(Task *task) {
    if (task == NULL) {
        return;
    }
    freeTasks(task->nextList);
    free(task);
    task = NULL;
    
    return;
}

void addTaskToList(TaskList *taskList, Task *task) {
    Task *aux = taskList->head;
    if (aux == NULL) {
        taskList->head = task;
        task->nextList = NULL;
        return;
    }
    while (aux->nextList != NULL) {
        aux = aux->nextList;
    }
    aux->nextList = task;
    task->nextList = NULL;
    return;
}

void createTask(TaskList *taskList, int period, int deadline, int burst) {
    Task *task = (Task *) malloc(sizeof(Task));
    if (task == NULL) {
        return;
    }
    task->period = period;
    task->deadline = deadline;
    task->burst = burst;

    task->timeFromStart = 0;
    task->timeFromExec = 0;

    task->lostDeadlinesCount = 0;
    task->completeExecutionCount = 0;
    task->killedCount = 0;

    task->nextList = NULL;
    task->nextQueue = NULL;
    addTaskToList(taskList, task);

    return;
}

void addTaskToQueue(TaskQueue *taskQueue, Task *task) {
    Task *aux = taskQueue->head;
    if (aux == NULL) {
        taskQueue->head = task;
        task->nextQueue = NULL;
        return;
    }
    while (aux->nextQueue != NULL) {
        aux = aux->nextQueue;
    }
    aux->nextQueue = task;
    task->nextQueue = NULL;

    return;
}

void removeTaskFromQueue(TaskQueue *taskQueue, Task *task) {
    Task *aux = taskQueue->head;
    if (aux == task) {
        taskQueue->head = task->nextQueue;
        return;
    }
    while (aux->nextQueue != task) {
        if (aux->nextQueue == NULL) {
            return;
        }
        aux = aux->nextQueue;
    }
    aux->nextQueue = task->nextQueue;

    return;
}

void killRemainingTasks(TaskQueue *taskQueue) {
    Task *aux = taskQueue->head;
    while (aux != NULL) {
        aux->killedCount++;
        aux = aux->nextQueue;
    }
    
    return;
}

void updateTaskQueue(TaskQueue *taskQueue, TaskList *taskList, int *totalTicks) {
    Task *aux = taskQueue->head;
    while (aux != NULL) {
        if (aux->timeFromStart == aux->deadline) {
            aux->lostDeadlinesCount++;
            removeTaskFromQueue(taskQueue, aux);
        }
        aux = aux->nextQueue;
    }
    aux = taskList->head;
    while (aux != NULL) {
        if ((*totalTicks) % aux->period == 0) {
            aux->timeFromStart = 0;
            aux->timeFromExec = 0;
            addTaskToQueue(taskQueue, aux);
        }
        aux = aux->nextList;
    }

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
        aux = aux->nextQueue;
    }

    return priority;
}

int waitForTask(TaskQueue *taskQueue, TaskList *taskList, int *totalTicks, int maxTicks) {
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
        updateTaskQueue(taskQueue, taskList, totalTicks);
    }
    if (currentCounter > 0) {
        printf("idle for %d units", currentCounter);
    }

    return code;
}

void randomBoringFunctionThatExistsToCallWaitForTaskBoringFunction(TaskQueue *taskQueue, TaskList* taskList, int *totalTicks, int maxTicks) {
    int code = waitForTask(taskQueue, taskList, totalTicks, maxTicks);

    if (code == 0) {
        killRemainingTasks(taskQueue);
    } else if (code == 1) {
        runTask(taskQueue, taskList, getMostPriority(taskQueue, maxTicks), totalTicks, maxTicks);
    }

    return;
}

void updateTimeFromStart(TaskQueue *taskQueue) {
    Task *aux = taskQueue->head;
    while (aux != NULL) {
        aux->timeFromStart++;
        aux = aux->nextQueue;
    }

    return;
}

void runTask(TaskQueue *taskQueue, TaskList *taskList, Task *task, int *totalTicks, int maxTicks) {
	int currentCounter = 0;
	char status = ' ';
	while (1) {
		// execute task...
		(*totalTicks)++;
        currentCounter++;
		task->timeFromExec++;
        updateTimeFromStart(taskQueue);
        updateTaskQueue(taskQueue, taskList, totalTicks);

        if (task->timeFromExec == task->burst) {
            task->completeExecutionCount++;
            removeTaskFromQueue(taskQueue, task);
            status = 'F';
            break;
        }
        if (task->timeFromStart == task->deadline) {
            status = 'L';
            break;
        }
        if ((*totalTicks) == maxTicks) {
            status = 'K';
            break;
        }
        if (task != getMostPriority(taskQueue, maxTicks)) {
            status = 'H';
            break;
        } 
	}
    // execution interrupted or finished
    task->timeFromExec = 0;
    printf("[%s] for %d units - %c\n", task->name, currentCounter, status);

    return;
}

int main(int argc, char **argv) {
    // period > deadline > burst

    return 0;
}