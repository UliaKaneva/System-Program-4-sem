#include "task1.h"

#define PARAMETERS_NUMBER 3
#define BUF_SIZE 50

Status cleanup(DB *db, User *user) {
    int st = save_db(db);
    delete_db(db);
    free(user);
    print_error_message(st);
    return st;
}

int main() {
    User *currentUser = (User *) calloc(1, sizeof(User));
    int isAuthorization = 0;
    int numberCommand = -1;
    if (!currentUser) {
        print_error_message(MemoryError);
        return MemoryError;
    }
    DB *dataBase;
    char filename[MAX_NAME_FILE] = "users.txt";
    int status = creat_db(filename, &dataBase);
    if (status != Success) {
        free(currentUser);
        print_error_message(status);
        return status;
    }

    while (1) {
        if (!isAuthorization) {
            int st = 0;
            switch (st = entry_menu(currentUser, dataBase)) {
                case Success:
                    isAuthorization = 1;
                    numberCommand = currentUser->numCommands;
                    break;
                default:
                    print_error_message(st);
                    return cleanup(dataBase, currentUser);
            }
        }
        printf("-To exit the console, enter \"Exit\"\n"
               "-To log out of your account, enter \"Logout\"\n");

        if (numberCommand != 0) {
            printf(
                    "-To find out the current console time, enter \"Time\"\n"
                    "-To find out the current date of the console, enter \"Date\"\n"
                    "-To find out how much time has passed since a specific date, "
                    "enter \"Howmuch <time> flag\" (<time> is entered in the "
                    "format dd:mm:yyyy; in \"flag\" enter -s if you want to get "
                    "the result in seconds, -m in minutes, -h in hours, -y in years)\n"
                    "-To limit the number of commands used by a user, enter \"Sanctions "
                    "username <number>\" (username is the name of the user you want "
                    "to limit; <number> is the number of commands that must be left to the user)\n"
            );
        }
        char command[BUF_SIZE] = "";
        read_str(command, BUF_SIZE);

        if (strcmp(command, "Exit") == 0) {
            print_error_message(Exit);
            break;
        }
        if (strcmp(command, "Logout") == 0) {
            isAuthorization = 0;
            continue;
        }
        if (numberCommand == 0) {
            print_error_message(InputError);
            continue;
        }
        if (strcmp(command, "Time") == 0) {
            if (print_time_or_date("%02d:%02d:%02d\n", 1)) {
                print_error_message(TimeError);
                continue;
            }
            if (numberCommand > 0) {
                --numberCommand;
            }
        } else if (strcmp(command, "Date") == 0) {
            if (print_time_or_date("%02d:%02d:%d\n", 0)) {
                print_error_message(TimeError);
                continue;
            }
            if (numberCommand > 0) {
                --numberCommand;
            }
        } else {
            char toHandler[PARAMETERS_NUMBER][14] = {"", "", ""};
            int indexWord = 0;
            char *token = strtok(command, " ");

            while (token != NULL && indexWord < PARAMETERS_NUMBER) {
                strncpy(toHandler[indexWord], token, 13);
                toHandler[indexWord][13] = '\0';
                indexWord++;
                token = strtok(NULL, " ");
            }
            if (token != NULL || indexWord != PARAMETERS_NUMBER) { // Параметров должно быть ровно 3
                print_error_message(InputError);
                continue;
            }
            if (strcmp(toHandler[0], "Howmuch") == 0) {
                double diff;
                int st = how_much(toHandler[1], toHandler[2], &diff);
                if (st != Success) {
                    print_error_message(st);
                    continue;
                }
                printf("%f\n", diff);
                numberCommand--;
            } else if (strcmp(toHandler[0], "Sanctions") == 0) {
                int st = sanctions(toHandler[1], toHandler[2], dataBase, currentUser);
                if (st != Success) {
                    print_error_message(st);
                    if (st == InputError) {
                        printf("No sanctions were applied to the user.\n");
                    }
                } else {
                    if (strcmp(toHandler[1], currentUser->login) == 0) {
                        numberCommand = currentUser->numCommands;
                    } else {
                        numberCommand--;
                    }
                }

            } else {
                print_error_message(InputError);
            }
        }
    }
    return cleanup(dataBase, currentUser);
}