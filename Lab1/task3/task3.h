#ifndef SIS_PROGA_1_TASK_TASK3_H
#define SIS_PROGA_1_TASK_TASK3_H

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

#define NUMBER_PHILOSOPHERS 10
typedef enum {
    Success,
    TokenError,
    SemError,
    MemoryError,
    NumberError

} Status;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

typedef struct {
    int id;
    int semid;
} Human;

void good(int *first, int *second, int id);

void *strategy(void *arg);

void bad(int *first, int *second, int id);

Status print_error_message(Status number_error);

#endif //SIS_PROGA_1_TASK_TASK3_H
