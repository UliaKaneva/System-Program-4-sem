#include "task4.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        return BadNumArguments;
    }
    FILE *file;
    FILE *file_out;
    file = fopen(argv[1], "r");
    if (!file) {
        return FileOpenError;
    }
    file_out = fopen("bed_res.txt", "w");
    if (!file_out) {
        fclose(file);
        return FileOpenError;
    }


    key_t key_req = ftok("tokfile", 55);
    if (key_req == -1) {
        fclose(file);
        fclose(file_out);
        return TokenFileError;
    }

    key_t key_res = ftok("tokfile", 56);
    if (key_res == -1) {
        fclose(file);
        fclose(file_out);
        return TokenFileError;
    }

    int req_queue = msgget(key_req, 0666);
    if (req_queue == -1) {
        fclose(file);
        fclose(file_out);
        return MessageQueueError;
    }

    int res_queue = msgget(key_res, 0666);
    if (res_queue == -1) {
        fclose(file);
        fclose(file_out);
        return MessageQueueError;
    }

    RequestMsg req;
    req.mtype = 1;
    pid_t client_pid = getpid();
    memcpy(req.data, &client_pid, sizeof(pid_t));

    char command[BUFFER_SIZE];
    Status status = Success;

    while (fgets(command, BUFFER_SIZE, file)) {
        command[strcspn(command, "\n")] = '\0';

        strcpy(req.data + sizeof(pid_t), command);
        if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
            fclose(file);
            fclose(file_out);
            return MessageSendError;
        }

        ResponseMsg res;
        if (msgrcv(res_queue, &res, sizeof(res.data), client_pid, 0) == -1) {
            fclose(file);
            fclose(file_out);
            return MessageReceiveError;
        }

        memcpy(&status, res.data, sizeof(Status));

        switch (status) {
            case Success:
                printf("Command '%s' executed successfully\n", command);
                break;
            case Win:
                printf("Congratulations! You successfully transported all items!\n");
                fclose(file);
                fclose(file_out);
                strcpy(req.data + sizeof(pid_t), "End");
                if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
                    fclose(file);
                    fclose(file_out);
                    return MessageSendError;
                }
                return Success;
            case Loss:
                printf("Game over! The goat ate the cabbage or the wolf ate the goat.\n");
                fclose(file);
                fclose(file_out);
                strcpy(req.data + sizeof(pid_t), "End");
                if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
                    fclose(file);
                    fclose(file_out);
                    return MessageSendError;
                }
                return Success;
            case EmptyBoot:
                printf("Error: Boat is empty. Cannot execute 'put' command.\n");
                break;
            case NotEmptyBoot:
                printf("Error: Boat is not empty. Cannot take another item.\n");
                break;
            case ObjectNotFound:
                printf("Error: Specified object not found on current shore.\n");
                break;
            case IncorrectCommand:
                printf("Error: Unknown command '%s'\n", command);
                break;
            default:
                printf("Error: Unknown status received from server\n");
                break;
        }
    }
    strcpy(req.data + sizeof(pid_t), "End");
    if (msgsnd(req_queue, &req, sizeof(req.data), 0) == -1) {
        fclose(file);
        fclose(file_out);
        return MessageSendError;
    }

    fclose(file);
    fclose(file_out);
    return Success;
}