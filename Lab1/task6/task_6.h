#ifndef SIS_PROGA_1_TASK_TASK_6_H
#define SIS_PROGA_1_TASK_TASK_6_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libgen.h>

typedef enum {
    Success,
    FileOpenError,
    BadNumArguments,
    TokenFileError,
    MessageQueueError,
    MessageSendError,
    NotCorrectPathInFile,
    MessageReceiveError,

    CountError

} Status;

#define MAX_MSG_SIZE 4088

typedef struct request_msg {
    long mtype;
    char data[MAX_MSG_SIZE];
} RequestMsg;

typedef struct response_msg {
    long mtype;
    char data[MAX_MSG_SIZE];
} ResponseMsg;

typedef struct file_list {
    char *file;
    struct file_list *next;
} FileList;

typedef struct directory {
    char *path;
    FileList *files;
    struct directory *next;
} Directory;

#endif //SIS_PROGA_1_TASK_TASK_6_H
