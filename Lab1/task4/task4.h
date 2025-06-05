#ifndef SIS_PROGA_1_TASK_TASK4_H
#define SIS_PROGA_1_TASK_TASK4_H

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 256

#define MAX_MSG_SIZE 4088

typedef enum {
    Success,
    End,
    Win,
    Loss,
    FileOpenError,
    BadNumArguments,
    TokenFileError,
    MessageQueueError,
    MessageSendError,
    NotCorrectPathInFile,
    MessageReceiveError,
    MemoryError,
    ClientNotFound,
    EmptyBoot,
    NotEmptyBoot,
    ObjectNotFound,
    IncorrectCommand,
    ForkError,
    CountError

} Status;

typedef enum {
    cabbage = 1,
    goat = 2,
    wolf = 4

} Object;

typedef struct request_msg {
    long mtype;
    char data[MAX_MSG_SIZE];
} RequestMsg;

typedef struct response_msg {
    long mtype;
    char data[MAX_MSG_SIZE];
} ResponseMsg;

typedef struct client_data { // Односвязный список клиентов
    pid_t client;
    unsigned int left; // Волк - 100, Коза - 010, Капуста - 001 в двоичной системе для left, right, boot
    unsigned int right;
    unsigned int boat;
    unsigned int boat_location;  // 0 - Лодка у левого берега, 1 - Лодка у правого берега
    struct client_data *next;

} Client; // По умолчанию все объекты находятся у левого берега, лодка пустая (только крестьянин) тоже у левого берега

#endif //SIS_PROGA_1_TASK_TASK4_H
