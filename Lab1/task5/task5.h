#ifndef SIS_PROGA_1_TASK_TASK5_H
#define SIS_PROGA_1_TASK_TASK5_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <pthread.h>
#include "errno.h"

#define EMPTY 0
#define WOMEN_ONLY 1
#define MEN_ONLY 2



typedef struct {
    int state;
    int women_count;
    int men_count;
    int max_people;
} bathroom_t;

typedef struct {
    bathroom_t *bathroom;
    int semid;
    int humenid;
}Arguments;

typedef enum {
    Success,
    InvalidNumber,
    SharedMemoryError,
    SemaphoreError,
    MemoryAllocationError

} Status;

#define SHM_KEY 1234
#define SEM_KEY 5678

#endif //SIS_PROGA_1_TASK_TASK5_H
