#ifndef SIS_PROGA_1_TASK_DATABASEUSERS_H
#define SIS_PROGA_1_TASK_DATABASEUSERS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_USERS 100
#define MAX_NAME_FILE 256


typedef struct {
    char login[6];
    unsigned int password;
    int numCommands;
} User;


typedef struct DataBase {
    char DataBaseFile[MAX_NAME_FILE];
    size_t count_users;
    User *users[MAX_USERS];

} DB;

typedef enum CodeStatus {
    // Successful
    Success,

    // Error
    MemoryError,
    OpenDataBaseError,
    NotUniqueLoginError,
    FullUsersListError,
    SaveDataBaseError,
    Exit,
    InputError,
    InvalidUsername,
    InvalidPassword,
    InvalidData,
    InvalidFlag,
    InvalidNumberSanction,
    UserNotExist,
    TimeError,
    Back,
    NumberError

} Status;

Status creat_db(char name_file[MAX_NAME_FILE], DB **dataBase);

User *search_user(DB *dataBase, char *login_user);

void delete_db(DB *dataBase);

Status save_db(DB *dataBase);

Status add_user(DB *dataBase, char name[6], unsigned int password);

void print_error_message(Status number_error);

#endif //SIS_PROGA_1_TASK_DATABASEUSERS_H
