#include "task3.h"



int main() {
    
    key_t key = ftok("./filetok", 'A');
    if (key == -1) {
        return print_error_message(TokenError);
    }
    int semid = semget(key, NUMBER_PHILOSOPHERS, IPC_CREAT | 0666);
    if (semid == -1) {
        return print_error_message(SemError);
    }

    unsigned short val[NUMBER_PHILOSOPHERS];
    for (int i = 0; i < NUMBER_PHILOSOPHERS; ++i) {
        val[i] = 1;
    }

    union semun init = {.array = val};

    if (semctl(semid, 0, SETALL, init) == -1) {
        semctl(semid, 0, IPC_RMID, 0);
        return print_error_message(SemError);
    }

    pthread_t threads[NUMBER_PHILOSOPHERS];

    for (int i = 0; i < NUMBER_PHILOSOPHERS; ++i) {
        Human* philosopher = (Human *) calloc(1, sizeof(Human));
        if (!philosopher) {
            for (int j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            semctl(semid, 0, IPC_RMID, 0);
            return print_error_message(MemoryError);
        }
        philosopher->id = i;
        philosopher->semid = semid;
        pthread_create(&threads[i], NULL, strategy, philosopher);
    }

    for (int j = 0; j < NUMBER_PHILOSOPHERS; ++j) {
        pthread_join(threads[j], NULL);
    }

//    for (int i = 0; i < NUMBER_PHILOSOPHERS; ++i){
//        printf("Философ %d поел %d раз\n", i, res[i]);
//    }
    semctl(semid, 0, IPC_RMID, 0);
    return 0;
}