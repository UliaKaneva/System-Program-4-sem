#ifndef SIS_PROGA_1_TASK_TASK2_H
#define SIS_PROGA_1_TASK_TASK2_H

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "strings.h"
#include <stdatomic.h>

typedef enum {
    Success,
    NumberArgumentsError,
    TypeArgumentsError,
    IncorrectFlagError,
    MemoryError,
    FileOpenError,
    WriteError,
    BigNumberError,
    ForkError,
    StringNotFind,
    CountError

} Status;

typedef enum {
    xorN,
    mask,
    copyN,
    find

} Flags;

typedef union Arguments {
    uint32_t N;
    char *SomeString;


} Args;

void error_print(Status stat);

Status flag_handler(int argc, char **argv, int *flag, Args *argFlag);

Status file_handler(char *filename, FILE **file, char *m);

Status xorN_handler(int argc, char **argv, Args *args);

Status mask_handler(int argc, char **argv, Args *args);

Status create_file_name(char *filename_in, int n, char *filename_out);

Status copy_f(FILE *from, FILE *to);

Status copy_files(char *filename, int N);

Status copy_handler(int argc, char **argv, Args *args);

Status count_suffiks(int **pi, String *str);

Status find_in_own_file(String *substring, char *name_file);

Status find_handler(int argc, char **argv, Args *args);

#endif //SIS_PROGA_1_TASK_TASK2_H
