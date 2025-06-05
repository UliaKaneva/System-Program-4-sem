#include "task4.h"


Client *find_client(Client *head, const pid_t pid) {
    Client *current = head;
    while (current) {
        if (current->client == pid) return current;
        current = current->next;
    }
    return NULL;
}

Status delete_client(Client **head, pid_t pid) {
    if (!head || !*head) return ClientNotFound;

    Client *current = *head;
    if (current->client == pid) {
        *head = current->next;
        free(current);
        return Success;
    }

    while (current->next) {
        if (current->next->client == pid) {
            Client *temp = current->next;
            current->next = temp->next;
            free(temp);
            return Success;
        }
        current = current->next;
    }
    return ClientNotFound;
}

Client *add_client(Client **head, pid_t pid) {
    Client *client = malloc(sizeof(Client));
    if (!client) {
        return NULL;
    }
    client->client = pid;
    client->left = 7;
    client->right = 0;
    client->boat = 0;
    client->boat_location = 0;
    client->next = *head;
    *head = client;
    return client;
}

Status answer_client(Client *client, Status stat, int res_queue) {
    ResponseMsg res;
    res.mtype = client->client;
    memcpy(res.data, &stat, sizeof(stat));
    if (msgsnd(res_queue, &res, sizeof(res.data), 0) == -1) {
        return MessageSendError;
    }
    return Success;
}

Status free_and_end_process(Client **head, Status stat, int res_queue) {
    Status s = Success;
    while (*head) {
        s = answer_client(*head, stat, res_queue);
        Client *temp = *head;
        *head = (*head)->next;
        free(temp);
    }
    return s;

}

Status take_handler(Client *client, Object ob) {
    if (client->boat_location == 0 && (client->left & ob)) {
        client->boat = ob;
        client->left ^= ob;
    } else if (client->boat_location == 1 && (client->right & ob)) {
        client->boat = ob;
        client->right ^= ob;
    } else {
        return ObjectNotFound;
    }
    return Success;
}

Status handler_commands(Client *client, char *command) {
    if (strcmp(command, "put") == 0) {
        if (client->boat == 0) {
            return EmptyBoot;
        }
        if (client->boat_location == 0) {
            client->left |= client->boat;
            client->boat = 0;
        } else {
            client->right |= client->boat;
            client->boat = 0;
        }
    } else if (strcmp(command, "move") == 0) {
        client->boat_location ^= 1;
    } else if (strncmp(command, "take ", 5) == 0) {
        if (client->boat) {
            return NotEmptyBoot;
        }
        char *ptr = command + 5;
        if (strcmp(ptr, "wolf") == 0) {
            if (take_handler(client, wolf)) {
                return ObjectNotFound;
            }
        } else if (strcmp(ptr, "cabbage") == 0) {
            if (take_handler(client, cabbage)) {
                return ObjectNotFound;
            }
        } else if (strcmp(ptr, "goat") == 0) {
            if (take_handler(client, goat)) {
                return ObjectNotFound;
            }
        } else {
            return IncorrectCommand;
        }
    } else {
        return IncorrectCommand;
    }
    if (client->right == 7) {
        return Win;
    }
    int res = client->boat_location ?  (client->left == 3 || client->left == 6) : (client->right == 3 || client->right == 6);
    if (res) {
        return Loss;
    }
    return Success;
}

int main() {
    key_t key_req = ftok("tokfile", 55);
    if (key_req == -1)
        return TokenFileError;

    int req_queue = msgget(key_req, IPC_CREAT | 0666);
    if (req_queue == -1)
        return MessageQueueError;

    key_t key_res = ftok("tokfile", 56);
    if (key_res == -1) {
        msgctl(req_queue, IPC_RMID, NULL);
        return TokenFileError;
    }


    int res_queue = msgget(key_res, IPC_CREAT | 0666);
    if (res_queue == -1) {
        msgctl(req_queue, IPC_RMID, NULL);
        return MessageQueueError;
    }
    Client *clients = NULL;

    while (1) {
        RequestMsg req;
        ssize_t received = msgrcv(req_queue, &req, sizeof(req.data), 1, 0);
        if (received == -1) {
            msgctl(req_queue, IPC_RMID, NULL);
            msgctl(res_queue, IPC_RMID, NULL);
            continue;
        }


        pid_t client_pid;
        memcpy(&client_pid, req.data, sizeof(pid_t));

        Client *current = find_client(clients, client_pid);
        if (!current) {
            current = add_client(&clients, client_pid);
            if (!current) {
                free_and_end_process(&clients, MessageQueueError, res_queue);
                msgctl(req_queue, IPC_RMID, NULL);
                msgctl(res_queue, IPC_RMID, NULL);
                return MessageQueueError;
            }
        }

        char *paths_data = req.data + sizeof(pid_t);
        if (strcmp(paths_data, "End") == 0) {
            if (delete_client(&clients, client_pid) == Success) {
                if (answer_client(current, End, res_queue)) {
                    free_and_end_process(&clients, MessageQueueError, res_queue);
                    msgctl(req_queue, IPC_RMID, NULL);
                    msgctl(res_queue, IPC_RMID, NULL);
                    return MessageQueueError;
                }
            }
            if (!clients) {
                struct msqid_ds buf;
                if (msgctl(req_queue, IPC_STAT, &buf) == -1) {
                    free_and_end_process(&clients, MessageQueueError, res_queue);
                    msgctl(req_queue, IPC_RMID, NULL);
                    msgctl(res_queue, IPC_RMID, NULL);

                    return MessageQueueError;
                }
                if (buf.msg_qnum == 0) {
                    free_and_end_process(&clients, End, res_queue);
                    msgctl(req_queue, IPC_RMID, NULL);
                    msgctl(res_queue, IPC_RMID, NULL);
                    return Success;
                }
            }
            continue;
        }
        Status res = handler_commands(current, paths_data);
        if (answer_client(current, res, res_queue)){
            free_and_end_process(&clients, MessageQueueError, res_queue);
            msgctl(req_queue, IPC_RMID, NULL);
            msgctl(res_queue, IPC_RMID, NULL);
            return MessageQueueError;
        }
        if (res == Loss || res == Win) {
            delete_client(&clients, client_pid);
        }

    }
}