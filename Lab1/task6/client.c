#include "task_6.h"



int main(int argc, char **argv) {
    if (argc != 2) {
        return BadNumArguments;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        return FileOpenError;
    }
    key_t key_req = ftok("tokfile", 55);
    if (key_req == -1) {
        fclose(file);
        return TokenFileError;
    }

    int req_queue = msgget(key_req, 0666);
    if (req_queue == -1) {
        fclose(file);
        return MessageQueueError;
    }

    key_t key_res = ftok("tokfile", 56);
    if (key_res == -1) {
        fclose(file);
        return TokenFileError;
    }

    int res_queue = msgget(key_res, 0666);
    if (res_queue == -1) {
        fclose(file);
        return MessageQueueError;
    }
    RequestMsg req = {.mtype = 1};
    pid_t pid = getpid();
    memcpy(req.data, &pid, sizeof(pid_t));

    char *ptr = req.data + sizeof(pid_t);
    size_t remaining = MAX_MSG_SIZE - sizeof(pid_t);
    int valid_paths = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        if (line[0] != '/') { // Путь не абсолютный
            continue;
        }
        struct stat st;
        if (stat(line, &st) == -1 || !S_ISREG(st.st_mode)) { // Путь не корректный или это не файл
            continue;
        }
        size_t len = strlen(line) + 1;
        if (remaining < len) { // Место в req кончилось
            break;
        }

        strcpy(ptr, line);
        ptr += len;
        remaining -= len;
        valid_paths++;
    }
    fclose(file);

    if (valid_paths == 0) {
        return NotCorrectPathInFile;
    }

    if (remaining >= 2) {
        *ptr++ = '\0';
        *ptr = '\0';
    }

    if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
        return MessageSendError;
    }

    ResponseMsg res;
    ssize_t bytes = msgrcv(res_queue, &res, sizeof(res.data), pid, 0);
    if (bytes == -1) {
        return MessageReceiveError;
    }

    char *data = res.data;
    while (*data) {
        printf("\nDirectory: %s\nFiles:", data);
        data += strlen(data) + 1;

        while (*data) {
            printf(" %s", data);
            data += strlen(data) + 1;
        }
        data++;
    }
    printf("\n");

    req.mtype = 1;
    pid = getpid();
    memcpy(req.data, &pid, sizeof(pid_t));
    ptr = req.data + sizeof(pid_t);
    strcpy(ptr, "Stop\0\0");
    if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
        return MessageSendError;
    }
    return Success;
}