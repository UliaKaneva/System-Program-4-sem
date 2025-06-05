#include "task3.h"

#define STRATEGY_USED good

Status print_error_message(Status number_error) {
    char errors[NumberError][150] =
            {
                    "Success\n",
                    "Token Error\n",
                    "Semaphore error\n",
                    "Memory allocation error\n"
            };
    fprintf(stderr, "%s", errors[number_error]);
    return number_error;
}

void *strategy(void *arg) {
    Human *philosopher = (Human *) arg;
    int id = philosopher->id;
    int semid = philosopher->semid;
    int count = NUMBER_PHILOSOPHERS;
    int first_fork;
    int second_fork;
    STRATEGY_USED(&first_fork, &second_fork, id);
    int i;

    for (i = 0; i < 1000; ++i) {

        struct sembuf op1 = {first_fork, -1, 0};
        semop(semid, &op1, 1);
        printf("Филосов %d взял вилку %d\n", id, first_fork);
        struct sembuf op2 = {second_fork, -1, 0};
        semop(semid, &op2, 1);
        printf("Филосов %d взял вилку %d\n", id, second_fork);
        printf("Филосов %d поел\n", id);

        struct sembuf op4 = {second_fork, 1, 0};
        semop(semid, &op4, 1);
        printf("Филосов %d положил вилку %d\n", id, second_fork);
        struct sembuf op3 = {first_fork, 1, 0};
        semop(semid, &op3, 1);
        printf("Филосов %d положил вилку %d\n", id, first_fork);

    }
    printf("\nФилосов %d поел %d раз\n\n", id, i);

    free(arg);
    return NULL;
}

void good(int *first, int *second, int id) {
    if (id > (id + 1) % NUMBER_PHILOSOPHERS) {
        *first = (id + 1) % NUMBER_PHILOSOPHERS;
        *second = id;
    } else {
        *first = id;
        *second = (id + 1) % NUMBER_PHILOSOPHERS;
    }
}

void bad(int *first, int *second, int id) {
    *first = id;
    *second = (id + 1) % NUMBER_PHILOSOPHERS;
}
