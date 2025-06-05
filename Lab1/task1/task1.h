#ifndef SIS_PROGA_1_TASK_TASK1_H
#define SIS_PROGA_1_TASK_TASK1_H

#include <ctype.h>
#include "DataBaseUsers.h"
#include "time.h"
#include <errno.h>
#include <limits.h>

int is_leap_year(int year);

int days(int year, int month, int day);

Status how_much(char time_str[14], const char flag[14], double *res);


void read_str(char *buf, int len);

void replace_char(char *str, char to, char with);

void copy_user(User *dest, User *src);

int str_to_upper(char *str);

Status authorization(User *user, DB *dataBase);

Status registration(User *user, DB *dataBase);

Status entry_menu(User *user, DB *dataBase);

Status sanctions(char username[14], char number[14], DB *dataBase, User *user);

Status print_time_or_date(const char *format, int is_time);

Status read_username(char login[8], DB *db, User **found_user);

Status read_password(char buf[8], unsigned long *password);

#endif //SIS_PROGA_1_TASK_TASK1_H
