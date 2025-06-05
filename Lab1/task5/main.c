#include "task5.h"

#define TOTAL_THREADS 10;

void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_signal(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void woman_wants_to_enter(bathroom_t *bathroom, int semid) {
    sem_wait(semid);

    while (bathroom->state == MEN_ONLY ||
           (bathroom->women_count + bathroom->men_count) >= bathroom->max_people) {
        sem_signal(semid);
        usleep(10000);
        sem_wait(semid);
    }

    if (bathroom->state == EMPTY) {
        bathroom->state = WOMEN_ONLY;
    }
    bathroom->women_count++;
    char ST[3][10] = {"EMPTY", "WOMEN_ONLY", "MEN_ONLY"};

    printf("Женщина зашла. Состояние: %s, женщин: %d, мужчин: %d\n",
           ST[bathroom->state], bathroom->women_count, bathroom->men_count);

    sem_signal(semid);
}

void man_wants_to_enter(bathroom_t *bathroom, int semid) {
    sem_wait(semid);

    while (bathroom->state == WOMEN_ONLY ||
           (bathroom->women_count + bathroom->men_count) >= bathroom->max_people) {
        sem_signal(semid);
        usleep(10000);
        sem_wait(semid);
    }

    if (bathroom->state == EMPTY) {
        bathroom->state = MEN_ONLY;
    }
    bathroom->men_count++;
    char ST[3][10] = {"EMPTY", "WOMEN_ONLY", "MEN_ONLY"};

    printf("Мужчина зашел. Состояние: %s, женщин: %d, мужчин: %d\n",
           ST[bathroom->state], bathroom->women_count, bathroom->men_count);

    sem_signal(semid);
}

void woman_leaves(bathroom_t *bathroom, int semid) {
    sem_wait(semid);

    bathroom->women_count--;
    if (bathroom->women_count == 0) {
        bathroom->state = EMPTY;
    }

    printf("Женщина вышла. Состояние: %s, женщин: %d, мужчин: %d\n",
           bathroom->state == WOMEN_ONLY ? "WOMEN_ONLY" :
           (bathroom->state == MEN_ONLY ? "MEN_ONLY" : "EMPTY"),
           bathroom->women_count, bathroom->men_count);

    sem_signal(semid);
}

void man_leaves(bathroom_t *bathroom, int semid) {
    sem_wait(semid);

    bathroom->men_count--;
    if (bathroom->men_count == 0) {
        bathroom->state = EMPTY;
    }

    printf("Мужчина вышел. Состояние: %s, женщин: %d, мужчин: %d\n",
           bathroom->state == WOMEN_ONLY ? "WOMEN_ONLY" :
           (bathroom->state == MEN_ONLY ? "MEN_ONLY" : "EMPTY"),
           bathroom->women_count, bathroom->men_count);

    sem_signal(semid);
}

void *woman_thread(void *arg) {
    Arguments *params = (Arguments *) arg;
    bathroom_t *bathroom = params->bathroom;
    int semid = params->semid;
    int id = params->humenid;

    printf("Женщина %d хочет войти\n", id);
    woman_wants_to_enter(bathroom, semid);
    sleep(1 + rand() % 4);
    printf("Женщина %d выходит\n", id);
    woman_leaves(bathroom, semid);

    free(arg);
    return NULL;
}

void *man_thread(void *arg) {
    Arguments *params = (Arguments *) arg;
    bathroom_t *bathroom = params->bathroom;
    int semid = params->semid;
    int id = params->humenid;

    printf("Мужчина %d хочет войти\n", id);
    man_wants_to_enter(bathroom, semid);
    sleep(1 + rand() % 3);
    printf("Мужчина %d выходит\n", id);
    man_leaves(bathroom, semid);
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return 1;
    }
    char *endPtr;
    errno = 0;
    int N = (int) strtol(argv[1], &endPtr, 10);
    if (*endPtr != '\0' || errno == ERANGE || N <= 0) {
        return InvalidNumber;
    }
    int total_threads = TOTAL_THREADS;
    srand(time(NULL));

    int shmid = shmget(SHM_KEY, sizeof(bathroom_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        return SharedMemoryError;
    }
    bathroom_t *bathroom = (bathroom_t *) shmat(shmid, NULL, 0);
    if (bathroom == (void *) -1) {
        shmctl(shmid, IPC_RMID, NULL);
        return SharedMemoryError;
    }
    bathroom->state = EMPTY;
    bathroom->women_count = 0;
    bathroom->men_count = 0;
    bathroom->max_people = N;

    // Создаем семафор
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        shmdt(bathroom);
        shmctl(shmid, IPC_RMID, NULL);
        return SemaphoreError;
    }
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        shmdt(bathroom);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return SemaphoreError;
    }
    pthread_t threads[total_threads];

    for (int i = 0; i < total_threads; i++) {
        Arguments *params = (Arguments *) calloc(1, sizeof(Arguments));
        if (!params) {
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            shmdt(bathroom);
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid, 0, IPC_RMID);
            return MemoryAllocationError;
        }
        params->bathroom = bathroom;
        params->semid = semid;
        params->humenid = i;
        if (rand() % 2 == 0) {
            pthread_create(&threads[i], NULL, woman_thread, params);
        } else {
            pthread_create(&threads[i], NULL, man_thread, params);
        }

        usleep(100000);
    }

    for (int i = 0; i < total_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    shmdt(bathroom);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}