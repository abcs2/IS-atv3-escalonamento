#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void freeLists(TaskList *taskList, TaskQueue *taskQueue) {
    freeTasks(taskList->head);
    free(taskList);
    free(taskQueue);
    taskList = NULL;
    taskQueue = NULL;

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

int createTask(TaskList *taskList, char *name, int period, int deadline, int burst) {
    Task *task = (Task *) malloc(sizeof(Task));
    if (task == NULL) {
        printf("Falha ao alocar memoria.\n");
        return -1;
    }
    strcpy(task->name, name);
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

    return 0;
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

int randomBoringFunctionThatExistsToCallWaitForTaskBoringFunction(TaskQueue *taskQueue, TaskList* taskList, int *totalTicks, int maxTicks) {
    int code = waitForTask(taskQueue, taskList, totalTicks, maxTicks);

    if (code == 0) {
        killRemainingTasks(taskQueue);
        return 0;
    } else if (code == 1) {
        runTask(taskQueue, taskList, getMostPriority(taskQueue, maxTicks), totalTicks, maxTicks);
    }

    return 1;
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

int readFile(TaskList *taskList, char *fileName) {
    int maxTicks, period, deadline, burst;
    char name[MAX_NAME];
    FILE *arq = fopen(fileName, "r");
    if (arq == NULL) {
        printf("Falha ao abrir o arquivo.\n");
        return -1;
    }
    fscanf(arq, "%d ", &maxTicks);
    if (maxTicks < 0) {
        printf("Tempo invalido de execucao.\n");
        fclose(arq);
        return -1;
    }
    while (fscanf("%s %d %d %d ", name, &period, &deadline, &burst) != -1) {
        if ((period < 0 || deadline < 0 || burst < 0) || (burst >= deadline || deadline >= period)) {
            printf("Tempo invalido de execucao.\n");
            fclose(arq);
            return -1;
        }
        if (createTask(taskList, name, period, deadline, burst) == -1) {
            fclose(arq);
            return -1;
        }
    }

    fclose(arq);
    return maxTicks;
}

void printResults(TaskList *taskList) {
    Task *aux = taskList->head;
    if (aux == NULL) {
        return;
    }
    printf("LOST DEADLINES\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->name, aux->lostDeadlinesCount);
        aux = aux->nextList;
    }
    printf("\n");

    aux = taskList->head;
    printf("COMPLETE EXECUTION\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->name, aux->completeExecutionCount);
        aux = aux->nextList;
    }
    printf("\n");

    aux = taskList->head;
    printf("KILLED\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->name, aux->killedCount);
        aux = aux->nextList;
    }

    return;
}

int main(int argc, char **argv) {
    // period > deadline > burst
    int rate_edf, maxTicks, totalTicks = 0;
    TaskList *taskList = (TaskList *) malloc(sizeof(TaskList));
    if (taskList == NULL) {
        printf("Falha ao alocar memoria.\n");
        return 1;
    }
    TaskQueue *taskQueue = (TaskQueue *) malloc(sizeof(TaskQueue));
    if (taskQueue == NULL) {
        printf("Falha ao alocar memoria.\n");
        free(taskList);
        taskList = NULL;
        return 1;
    }
    taskList->head = NULL;
    taskQueue->head = NULL;

    if (argc != 3) {
        printf("Quantidade invalida de argumentos.\n");
        freeLists(taskList, taskQueue);
        return 1;
    }
    if (strcmp(argv[1], "rate") == 0) {
        rate_edf = 0;
    } else if (strcmp(argv[1], "edf") == 0) {
        rate_edf = 1;
    } else {
        printf("Tipo de escalonador invalido.\n");
        freeLists(taskList, taskQueue);
        return 1;
    }

    maxTicks = readFile(taskList, argv[2]);
    if (maxTicks == -1) {
        freeLists(taskList, taskQueue);
        return -1;
    }
    updateTaskQueue(taskQueue, taskList, &totalTicks);
    while (randomBoringFunctionThatExistsToCallWaitForTaskBoringFunction(taskQueue, taskList, &totalTicks, maxTicks) != 0);
    printResults(taskList);

    freeLists(taskList, taskQueue);
    return 0;
}