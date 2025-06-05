#include "task1.h"


void replace_char(char *str, char to, char with) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == to) {
            str[i] = with;
        }
    }
}

Status authorization(User *user, DB *dataBase) {
    User *person;
    while (1) {
        printf("Enter your username or \"<-\" to exit the authorization menu:\n");
        char login[8] = "";
        int st = read_username(login, dataBase, &person);
        if (st != Success && st != InvalidUsername){
            return st;
        }
        if (st == InvalidUsername){
            print_error_message(InvalidUsername);
            continue;
        }
        if (!person) {
            print_error_message(UserNotExist);
            continue;
        }
        break;
    }
    while (1) {
        printf("Enter password:\n");
        unsigned long password;
        char buf[8] = "";
        read_str(buf, 8);
        if (!strcmp(buf, "Exit")) {
            return Exit;
        }
        if (!strcmp(buf, "<-")){
            return Back;
        }
        if (read_password(buf, &password) == InvalidPassword || password != person->password){
            print_error_message(InvalidPassword);
            continue;
        }
        break;
    }
    copy_user(user, person);
    return Success;
}

int str_to_upper(char *str) {
    if (strlen(str) > 6 || strlen(str) == 0) {
        return 1;
    }
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = (char) toupper((unsigned char) str[i]);
        if (!isalnum(str[i])) {
            return 1;
        }
    }

    return 0;
}

void read_str(char *buf, int len) {
    int c;
    int i = 0;
    while ((c = getchar()) != '\n' && c != EOF && i < (len - 1)) {
        buf[i++] = (char) c;
    }
    buf[i] = '\0';
    if (c != EOF && c != '\n') {
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

Status read_username(char login[8], DB *db, User **found_user) {
    read_str(login, 8);
    if (strcmp(login, "Exit") == 0) {
        return Exit;
    }
    if (strcmp(login, "<-") == 0) {
        return Back;
    }
    if (str_to_upper(login)) {
        return InvalidUsername;
    }
    *found_user = search_user(db, login);
    return Success;
}

Status read_password(char buf[8], unsigned long *password){
    char *endPtr;
    replace_char(buf, ' ', '-');
    *password = strtoul(buf, &endPtr, 10);
    if (buf[0] == '\0' || *endPtr != '\0' || *password > 100000) {
        return InvalidPassword;
    }
    return Success;
}

Status registration(User *user, DB *dataBase) {
    if (dataBase->count_users == MAX_USERS) {
        return FullUsersListError;
    }
    User *person;
    while (1) {
        printf("Enter your new username or \"<-\" to exit the authorization menu:\n");
        char login[8] = "";
        int st = read_username(login, dataBase, &person);
        if (st == InvalidUsername) {
            print_error_message(InvalidUsername);
            continue;
        }
        if (st != Success) {
            return st;
        }
        if (person) {
            print_error_message(NotUniqueLoginError);
            continue;
        }
        strcpy(user->login, login);
        break;
    }
    while (1) {
        printf("Enter password:\n");
        unsigned long password;
        char buf[8] = "";

        read_str(buf, 8);
        if (!strcmp(buf, "Exit")) {
            return Exit;
        }
        if (!strcmp(buf, "<-")){
            return Back;
        }
        if (read_password(buf, &password) == InvalidPassword){
            print_error_message(InvalidPassword);
            continue;
        }
        user->password = password;
        break;
    }
    user->numCommands = -1;
    return add_user(dataBase, user->login, user->password);

}


Status entry_menu(User *user, DB *dataBase) {
    while (1) {
        int st = 0;
        printf("Authorization is required to use the console.\n"
               "To log in to your username, type 1\n"
               "To register a new username, type 2\n"
               "To exit the console, type \"Exit\"\n");

        char ans[100];
        read_str(ans, 100);
        if (strcmp(ans, "Exit") == 0) {
            return Exit;
        }
        if (strcmp(ans, "2") == 0) {
            st = registration(user, dataBase);
            if (st == FullUsersListError) {
                print_error_message(FullUsersListError);
            }
            if (st == Back) {
                continue;
            }
            return st;
        }
        if (strcmp(ans, "1") == 0) {
            st = authorization(user, dataBase);
            if (st == Back) {
                continue;
            }
            return st;
        } else {
            printf("Invalid command! Try again.\n");
        }

    }
}

void copy_user(User *dest, User *src) {
    dest->password = src->password;
    dest->numCommands = src->numCommands;
    for (int i = 0; i < 6; i++) {
        dest->login[i] = src->login[i];
    }
}

Status how_much(char *time_str, const char *flag, double *res) {
    int day;
    int month;
    int year;
    int day_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (strlen(time_str) != 10) {
        return InvalidData;
    }
    if (sscanf(time_str, "%d:%d:%d",
               &day, &month, &year) != 3) {
        return InvalidData;
    }
    if (year <= 0) {
        return InvalidData;
    }
    day_month[1] += (is_leap_year(year)) ? 1 : 0;
    if (month <= 0 || month > 12 || day <= 0 || day > day_month[month - 1]) {
        return InvalidData;
    }
    int time1 = days(year, month, day);
    time_t time_2 = time(NULL);
    struct tm *tmp = localtime(&time_2);
    int time2 = days(tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday);
    double diff_seconds = (((double) (time2 - time1) * 24 + tmp->tm_hour) * 60 + tmp->tm_min) * 60 + tmp->tm_sec;
    if (diff_seconds < 0) {
        return InvalidData;
    }
    if (flag[0] != '-' || flag[2] != '\0') {
        return InvalidFlag;
    }
    switch (flag[1]) {
        case 's':
            *res = diff_seconds;
            return Success;
        case 'm':
            *res = diff_seconds / 60;
            return Success;
        case 'h':
            *res = diff_seconds / 3600;
            return Success;
        case 'y':
            *res = diff_seconds / 31536000;
            return Success;
        default:
            return InvalidFlag;
    }
}

int days(int year, int month, int day) {
    int day_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    day_month[1] += (is_leap_year(year)) ? 1 : 0;
    int time1 = (year / 4 - year / 100 + year / 400) + year * 365;
    for (int i = 0; i < month - 1; i++) {
        time1 += day_month[i];
    }
    time1 += day;
    return time1;
}

Status sanctions(char *username, char *number, DB *dataBase, User *user) {
    if (str_to_upper(username)) {
        return InvalidUsername;
    }
    char *endPtr;
    long int num_s = strtol(number, &endPtr, 10);
    if (*endPtr != '\0' || errno == ERANGE || num_s < -1 || num_s > INT_MAX) {
        return InvalidNumberSanction;
    }
    User *user_to_sanction = search_user(dataBase, username);
    if (!user_to_sanction) {
        return UserNotExist;
    }
    printf("Enter \"12345\" to correct the change\n");
    char confirmation[8];
    read_str(confirmation, 8);
    if (strcmp(confirmation, "12345") != 0) {
        return InputError;
    }
    if (strcmp(user_to_sanction->login, user->login) == 0) {
        user->numCommands = (int) num_s;
    }
    user_to_sanction->numCommands = (int) num_s;
    return Success;
}

int is_leap_year(int year) {
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}

Status print_time_or_date(const char *format, int is_time) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (!tm) {
        return TimeError;
    }
    printf(format,
           is_time ? tm->tm_hour : tm->tm_mday,
           is_time ? tm->tm_min : tm->tm_mon + 1,
           is_time ? tm->tm_sec : tm->tm_year + 1900);
    return Success;
}
