#include "DataBaseUsers.h"

Status creat_db(char name_file[MAX_NAME_FILE], DB **dataBase) {
    *dataBase = (DB *) calloc(1, sizeof(DB));
    if (!*dataBase) {
        return MemoryError;
    }
    strcpy((*dataBase)->DataBaseFile, name_file);

    FILE *file_creat = fopen(name_file, "rb");
    if (!file_creat) {
        file_creat = fopen(name_file, "wb");
        if (!file_creat) {
            free(*dataBase);
            return OpenDataBaseError;
        }
        (*dataBase)->count_users = 0;

        if (fwrite(&((*dataBase)->count_users), sizeof(int), 1, file_creat) != 1) {
            fclose(file_creat);
            free(*dataBase);
            return OpenDataBaseError;
        }

        fclose(file_creat);
        return Success;
    }
    if (fread(&((*dataBase)->count_users), sizeof(int), 1, file_creat) != 1) {
        fclose(file_creat);
        free(*dataBase);
        return OpenDataBaseError;
    }
    for (int i = 0; i < (*dataBase)->count_users; i++) {
        User *user = (User *) calloc(1, sizeof(User));
        if (!user || fread(user, sizeof(User), 1, file_creat) != 1) {
            fclose(file_creat);
            delete_db(*dataBase);
            return OpenDataBaseError;
        }
        (*dataBase)->users[i] = user;
    }

    fclose(file_creat);
    return Success;
}

void delete_db(DB *dataBase) {
    if (!dataBase) return;
    for (int i = 0; i < dataBase->count_users; i++) {
        free(dataBase->users[i]);
    }
    free(dataBase);
}

User *search_user(DB *dataBase, char *login_user) {
    for (int i = 0; i < dataBase->count_users; i++) {
        if (!strcmp(dataBase->users[i]->login, login_user)) {
            return dataBase->users[i];
        }
    }
    return NULL;
}

Status save_db(DB *dataBase) {
    FILE *file_to_save = fopen(dataBase->DataBaseFile, "wb");
    if (!file_to_save) {
        return SaveDataBaseError;
    }
    if (fwrite(&(dataBase->count_users), sizeof(int), 1, file_to_save) != 1) {
        fclose(file_to_save);
        return SaveDataBaseError;
    }
    for (int i = 0; i < dataBase->count_users; i++) {
        if (fwrite(dataBase->users[i], sizeof(User),
                   1, file_to_save) != 1) {
            fclose(file_to_save);
            return SaveDataBaseError;
        }
    }

    fclose(file_to_save);
    return Success;

}

Status add_user(DB *dataBase, char name[6], unsigned int password) {
    if (search_user(dataBase, name)) {
        return NotUniqueLoginError;
    }
    if (dataBase->count_users == MAX_USERS) {
        return FullUsersListError;
    }
    dataBase->users[dataBase->count_users] = (User *) calloc(1, sizeof(User));
    if (!dataBase->users[dataBase->count_users]) {
        return MemoryError;
    }
    strcpy(dataBase->users[dataBase->count_users]->login, name);
    dataBase->users[dataBase->count_users]->password = password;
    dataBase->users[dataBase->count_users]->numCommands = -1;
    dataBase->count_users += 1;
    return Success;
}

void print_error_message(Status number_error) {
    char errors[NumberError][150] =
            {
                    "Success\n",
                    "Memory allocation error\n",
                    "Error opening the data file\n",
                    "Such a user exists. Come up with a different username or log in.\n",
                    "The database is full, it is not possible to create a new login.\n",
                    "Saving error, recent data has not been saved\n",
                    "Exiting...\n",
                    "Input Error. Try to enter again\n",
                    "The username must contain only Latin letters and numbers and should be no longer than 6 characters\n",
                    "The password must be an integer, not a negative number, not more than 100,000.\n",
                    "Date entry error, make sure that you enter the date in the format \"dd:mm:yyyy\", the date must be earlier than the current time\n",
                    "An unknown flag. \"-s\" is the result in seconds, \"-m\" is the result in minutes, \"-h\" is the result in hours, \"-y\" is the result in years.\n",
                    "Incorrect number of available commands\n",
                    "The user was not found\n",
                    "Time Error\n"
            };
    printf("%s", errors[number_error]);
}