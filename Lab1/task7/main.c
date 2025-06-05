#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#define MAX_PATH 1024

typedef enum {
    Success,
    OpenDirError,
    ReadDirError,
    LengthPath_Error,
    LstatError

} Status;


Status print_file_info(const char *dir_name, const char *file_name) {
    struct stat file_stat;
    char full_path[MAX_PATH];

    // Формируем полный путь к файлу
    int wright = snprintf(full_path, sizeof(full_path), "%s/%s", dir_name, file_name);
    if (wright >= MAX_PATH) {
        return LengthPath_Error;
    }
    if (lstat(full_path, &file_stat) < 0) {
        return LstatError;
    }
    char type;
    switch (file_stat.st_mode & S_IFMT) {
        case S_IFREG:
            type = '-';
            break;  // Обычный файл
        case S_IFDIR:
            type = 'd';
            break;  // Каталог
        case S_IFLNK:
            type = 'l';
            break;  // Символическая ссылка
        case S_IFBLK:
            type = 'b';
            break;  // Блочное устройство
        case S_IFCHR:
            type = 'c';
            break;  // Символьное устройство
        case S_IFIFO:
            type = 'p';
            break;  // FIFO (именованный канал)
        case S_IFSOCK:
            type = 's';
            break;  // Сокет
        default:
            type = '?';
            break;  // Неизвестный тип
    }

    char permissions[] = "---------";
    if (file_stat.st_mode & S_IRUSR) { permissions[0] = 'r'; }
    if (file_stat.st_mode & S_IWUSR) { permissions[1] = 'w'; }
    if (file_stat.st_mode & S_IXUSR) { permissions[2] = 'x'; }
    if (file_stat.st_mode & S_IRGRP) { permissions[3] = 'r'; }
    if (file_stat.st_mode & S_IWGRP) { permissions[4] = 'w'; }
    if (file_stat.st_mode & S_IXGRP) { permissions[5] = 'x'; }
    if (file_stat.st_mode & S_IROTH) { permissions[6] = 'r'; }
    if (file_stat.st_mode & S_IWOTH) { permissions[7] = 'w'; }
    if (file_stat.st_mode & S_IXOTH) { permissions[8] = 'x'; }

    // Владелец и группа
    struct passwd *pwd = getpwuid(file_stat.st_uid);
    struct group *grp = getgrgid(file_stat.st_gid);
    const char *owner = (pwd == NULL) ? pwd->pw_name : "?";
    const char *group = (grp == NULL) ? grp->gr_name : "?";
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

    printf(
            "%c%s %4lu %8s %8s %8lld %s %s\n",
            type, permissions, file_stat.st_nlink, owner, group,
            (long long) file_stat.st_size, time_str, file_name
    );
    return Success;
}

Status list_directory(const char *dir_name) {
    DIR *dir;


    dir = opendir(dir_name);
    if (!dir) {
        return OpenDirError;
    }
    printf("Directory: %s\n", dir_name);
    printf("Type Perms Links Owner    Group    Size      Modified             Name\n");
    printf("----------------------------------------------------------------------\n");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        int st = print_file_info(dir_name, entry->d_name);
        if (st != Success){
            return st;
        }
    }
    if (errno) {
        closedir(dir);
        return ReadDirError;
    }

    closedir(dir);
    printf("\n");
    return Success;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        int st = list_directory(".");
        if (st != Success){
            return st;
        }
    } else {
        for (int i = 1; i < argc; i++) {
            int st = list_directory(argv[i]);
            if (st != Success){
                return st;
            }
        }
    }
    return Success;
}