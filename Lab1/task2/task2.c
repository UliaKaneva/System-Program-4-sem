#include "task2.h"

void error_print(Status stat) {
    char message_error[CountError][100] = {
            "Success",
            "Insufficient number of arguments",
            "Unsuitable type of flag argument",
            "Incorrect flag or missing flag",
            "Memory allocation error",
            "Couldn't open the file",
            "Couldn't write to a file",
            "The number is too high",
            "Failed to create child process",
            "This substring is not present in this file."
    };
    printf("%s\n", message_error[stat]);

}

Status flag_handler(int argc, char **argv, int *flag, Args *argFlag) {
    if (strncmp(argv[argc - 1], "xorN", 3) == 0) {
        if (strlen(argv[argc - 1]) == 4 && argv[argc - 1][3] <= '6' && argv[argc - 1][3] >= '2') {
            argFlag->N = argv[argc - 1][3] - '0';
            *flag = xorN;
            return Success;
        }
        return TypeArgumentsError;
    } else if (strncmp(argv[argc - 1], "copyN", 4) == 0) {
        unsigned long len_num = strlen(argv[argc - 1]);
        if (len_num <= 7 && len_num >= 5) {
            int num = 0;
            for (int i = 4; i < len_num; i++) {
                if (!isdigit(argv[argc - 1][i])) {
                    return TypeArgumentsError;
                }
                num *= 10;
                num += argv[argc - 1][i] - '0';

            }
            if (num > 10) {
                return BigNumberError;
            }
            *flag = copyN;
            argFlag->N = num;
            return Success;
        }
        return TypeArgumentsError;
    } else if (strncmp(argv[argc - 2], "mask", 4) == 0) {
        if (argc < 4) {
            return NumberArgumentsError;
        }
        char *endptr;
        errno = 0;
        long mask_num = strtol(argv[argc - 1], &endptr, 16);

        if (argv[argc - 1] == endptr || *endptr != '\0' || errno) {
            return TypeArgumentsError;
        }
        *flag = mask;
        argFlag->N = mask_num;
        return Success;
    } else if (strncmp(argv[argc - 2], "find", 4) == 0) {
        if (argc < 4) {
            return NumberArgumentsError;
        }
        *flag = find;
        argFlag->SomeString = argv[argc - 1];
        return Success;
    }
    return IncorrectFlagError;
}

Status file_handler(char *filename, FILE **file, char *m) {
    *file = fopen(filename, m);
    if (!*file) {
        return FileOpenError;
    }
    return Success;
}

Status xorN_handler(int argc, char **argv, Args *args) {
    uint32_t N = args->N;
    size_t block_size = (N == 2) ? 1 : (1 << (N - 3));
    uint8_t *block = malloc(block_size);
    if (!block) {
        return MemoryError;
    }
    uint8_t *xor_result = malloc(block_size);
    if (!xor_result) {
        free(block);
        return MemoryError;
    }
    for (int i = 1; i < argc - 1; i++) {
        FILE *file;
        if (file_handler(argv[i], &file, "rb") == FileOpenError) {
            printf("%s ", argv[i]);
            error_print(FileOpenError);
            continue;
        }
        memset(block, 0, block_size);
        memset(xor_result, 0, block_size);

        size_t bytes_read;
        while ((bytes_read = fread(block, 1, block_size, file)) > 0) {
            if (bytes_read < block_size) {
                memset(block + bytes_read, 0, block_size - bytes_read);
            }
            if (N > 2) {
                for (size_t j = 0; j < block_size; j++) {
                    xor_result[j] ^= block[j];
                }
            }
            else {
                xor_result[0] ^= (block[0] >> 2) ^ (block[0] & 15);
            }

        }
        unsigned long long resalt = 0;
        for (size_t j = 0; j < block_size; j++) {
            resalt <<= 8;
            resalt |= xor_result[j];
        }
        printf("%s: %lld\n", argv[i], resalt);
        fclose(file);
    }
    free(block);
    free(xor_result);
    return Success;
}

Status mask_handler(int argc, char **argv, Args *args) {
    uint32_t int_mask = args->N;
    for (int i = 1; i < argc - 2; i++) {
        FILE *file;
        if (file_handler(argv[i], &file, "r") == FileOpenError) {
            printf("%s ", argv[i]);
            error_print(FileOpenError);
            continue;
        }
        size_t sum_numbers = 0;
        int c;
        uint32_t current_number = 0;
        int in_num = 0;
        while ((c = fgetc(file)) != EOF) {
            if (isdigit(c)) {
                if (!in_num) {
                    current_number = c - '0';
                    in_num = 1;
                } else {
                    current_number *= 10;
                    current_number += c - '0';
                }
            } else {
                if (in_num) {
                    if (current_number == int_mask) {
                        sum_numbers++;
                    }
                    in_num = 0;
                }
            }
        }
        if (in_num && current_number == int_mask) {
            sum_numbers++;
        }
        printf("%s: %zu\n", argv[i], sum_numbers);
        fclose(file);
    }
    return Success;
}

Status create_file_name(char *filename_in, int n, char *filename_out) {
    if (!filename_in || !filename_out) {
        return TypeArgumentsError;
    }
    int len = (int) strlen(filename_in);
    int k = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (filename_in[i] == '.') {
            k = i;
            break;
        }
    }

    if (k == -1) {
        sprintf(filename_out, "%s_%d", filename_in, n);
    } else {
        char tmp1[len];
        char tmp2[len];
        for (int i = 0; i < len; i++) {
            if (i < k) {
                tmp1[i] = filename_in[i];
            } else {
                tmp2[i - k] = filename_in[i];
            }
        }
        tmp1[k] = '\0';
        tmp2[len - k] = '\0';
        sprintf(filename_out, "%s_%d%s", tmp1, n, tmp2);
    }

    return Success;
}

Status copy_f(FILE *from, FILE *to) {
    if (from == NULL || to == NULL) {
        return TypeArgumentsError;
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), from)) > 0) {
        size_t n = fwrite(buffer, 1, bytes, to);
        if (n != bytes) {
            return WriteError;
        }
    }
    return Success;
}

Status copy_files(char *filename, int N) {
    FILE *file_src;
    if (file_handler(filename, &file_src, "rb") == FileOpenError) {
        return FileOpenError;
    }
    char *filename_dst = (char *) calloc(strlen(filename) + (unsigned long) ceil(log10((double) (N + 1))) + 1,
                                         sizeof(char));
    if (!filename_dst) {
        fclose(file_src);
        return MemoryError;
    }
    for (int i = 0; i < N; i++) {
        Status stat = create_file_name(filename, i + 1, filename_dst);
        if (stat) {
            free(filename_dst);
            fclose(file_src);
            return stat;
        }
        FILE *file_dst;
        if (file_handler(filename_dst, &file_dst, "wb") == FileOpenError) {
            free(filename_dst);
            fclose(file_src);
            return FileOpenError;
        }
        stat = copy_f(file_src, file_dst);
        if (stat) {
            free(filename_dst);
            fclose(file_src);
            fclose(file_dst);
            return stat;
        }
        fclose(file_dst);
        rewind(file_src);
    }
    free(filename_dst);
    fclose(file_src);
    return Success;

}

Status copy_handler(int argc, char **argv, Args *args) {
    uint32_t N = args->N;
    int num_files = argc - 2;
    pid_t *pids = malloc(num_files * sizeof(pid_t));
    if (!pids) {
        return MemoryError;
    }
    for (int i = 1; i <= num_files; i++) {
        pids[i - 1] = fork();
        if (pids[i - 1] < 0) {
            for (int j = 0; j < i - 1; j++) {
                kill(pids[j], SIGTERM);
            }
            free(pids);
            return ForkError;
        } else if (pids[i - 1] == 0) {
            Status stat = copy_files(argv[i], N);
            switch (stat) {
                case MemoryError:
                    for (int j = 0; j < i - 1; j++) {
                        kill(pids[j], SIGTERM);
                    }
                    free(pids);
                    return MemoryError;
                case FileOpenError:
                    error_print(FileOpenError);
                    break;
                default:
                    break;
            }
        }
    }
    for (int i = 0; i < num_files; i++) {
        waitpid(pids[i], NULL, 0);
    }

    free(pids);
    return Success;
}

Status count_suffiks(int **pi, String *str) {
    *pi = (int *) calloc(str->length, sizeof(int));
    if (*pi == NULL) {
        return MemoryError;
    }
    int j = 0;
    int i = 1;
    (*pi)[0] = 0;
    while (i < str->length) {
        if (str->data[j] == str->data[i]) {
            (*pi)[i] = j + 1;
            ++i;
            ++j;
        } else {
            if (!j) {
                (*pi)[i] = 0;
                ++i;
            } else {
                j = (*pi)[j - 1];
            }
        }
    }
    return Success;
}

Status find_in_own_file(String *substring, char *name_file) {
    FILE *file;
    if (file_handler(name_file, &file, "r") == FileOpenError) {
        error_print(FileOpenError);
        return FileOpenError;
    }
    int *pi;
    if (count_suffiks(&pi, substring)) {
        fclose(file);
        return MemoryError;
    }
    int index_buf = 0;
    char c;
    while ((c = (char) fgetc(file)) != EOF) {
        if (c == substring->data[index_buf]) {
            ++index_buf;
            if (index_buf == substring->length) {
                fclose(file);
                free(pi);
                return Success;
            }
        } else {
            while (index_buf != 0 && substring->data[index_buf] != c) {
                index_buf = pi[index_buf - 1];
            }
            if (substring->data[index_buf] == c) {
                ++index_buf;
            }
        }
    }
    fclose(file);
    free(pi);
    if (index_buf == substring->length) {
        return Success;
    }
    return StringNotFind;
}

Status find_handler(int argc, char **argv, Args *args) {
    char *substr = args->SomeString;
    int num_files = argc - 3;

    String *substring = create(substr);
    if (substring == NULL) {
        return MemoryError;
    }

    pid_t *pids = malloc(num_files * sizeof(pid_t));
    if (!pids) {
        delete(substring);  // Освобождаем substring перед возвратом
        return MemoryError;
    }

    for (int i = 1; i <= num_files; ++i) {
        pids[i - 1] = fork();
        if (pids[i - 1] < 0) {
            // При ошибке fork освобождаем все ресурсы
            for (int j = 0; j < i - 1; j++) {
                kill(pids[j], SIGTERM);
            }
            free(pids);
            delete(substring);
            return ForkError;
        }
        else if (pids[i - 1] == 0) {
            Status stat = find_in_own_file(substring, argv[i]);
            if (stat == MemoryError) {
                delete(substring);
                free(pids);
                exit(MemoryError);
            }
            else if (stat == Success) {
                printf("\"%s\" in %s\n", substr, argv[i]);
            }
            delete(substring);
            free(pids);
            exit(stat);
        }
    }

    // В родительском процессе
    int status;
    int res = 0;
    for (int i = 0; i < num_files; i++) {
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            res = res || Success == WEXITSTATUS(status);
        }
    }

    free(pids);
    delete(substring);
    return res ? Success : StringNotFind;
}

//Status find_handler(int argc, char **argv, Args *args) {
//    char *substr = args->SomeString;
//    int num_files = argc - 2;
//    atomic_int *flag = 0;
//    String *substring = create(substr);
//    if (substring == NULL) {
//        return MemoryError;
//    }
//    pid_t *pids = malloc(num_files * sizeof(pid_t));
//    if (!pids) {
//        delete(substring);
//        return MemoryError;
//    }
//    for (int i = 1; num_files > i; ++i) {
//        pids[i - 1] = fork();
//        if (pids[i - 1] < 0) {
//            for (int j = 0; j < i - 1; j++) {
//                kill(pids[j], SIGTERM);
//            }
//            delete(substring);
//            free(pids);
//            return ForkError;
//        } else if (pids[i - 1] == 0) {
//            Status stat = find_in_own_file(substring, argv[i]);
//            if (stat == MemoryError) {
//                for (int j = 0; j < i - 1; j++) {
//                    kill(pids[j], SIGTERM);
//                }
//                delete(substring);
//                free(pids);
//
//                return MemoryError;
//            } else if (stat == Success) {
//                atomic_store(flag, 1);
//                printf("%s in %s\n", substr, argv[i]);
//            }
//        }
//    }
//    for (int i = 0; i < num_files; i++) {
//        waitpid(pids[i], NULL, 0);
//    }
//    free(pids);
//    delete(substring);
//    return Success;
//}
