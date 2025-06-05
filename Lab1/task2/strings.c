#include "strings.h"



int save_add_char(char **array, int *size, int i, char element) {
    if (i >= *size) {
        *size *= 2;
        char *temp = (char *) realloc(*array, *size * sizeof(char));
        if (temp == NULL) {
            return 1;
        }
        *array = temp;
    }
    (*array)[(i)] = (element);
    return 0;
}

String *create(char *str) {
    if (!str) {
        return NULL;
    }
    String *res = (String *) calloc(1, sizeof(String));
    int size = 2;
    if (!res) {
        return NULL;
    }
    res->data = (char *) calloc(size, sizeof(char));
    if (!(res->data)) {
        free(res);
        return NULL;
    }
    int index;
    for (index = 0; str[index]; index++) {
        if (save_add_char(&(res->data), &size, index, str[index])) {
            free(res->data);
            free(res);
            return NULL;
        }
    }
    if (save_add_char(&(res->data), &size, index, '\0')) {
        free(res->data);
        free(res);
        return NULL;
    }
    res->length = index;
    res->size_data = size;
    return res;
}

void clear(String *str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->length = 0;
        str->size_data = 0;
    }
}

int add(String *str, char to_add){
    str->data[str->length] = to_add;
    str->length += 1;
    if (save_add_char(&(str->data), &(str->size_data), str->length, '\0')){
        return 1;
    }
    return 0;
}

int compare(const void *s1, const void *s2) {
    String *str1 = (String *) s1;
    String *str2 = (String *) s2;
    if (!str1 && !str2) {
        return 0;
    }
    if (!str1) {
        return -1;
    }
    if (!str2) {
        return 1;
    }
    if (str1->length > str2->length) {
        return 1;
    }
    if (str1->length < str2->length) {
        return -1;
    }
    for (int i = 0; i < str1->length; i++) {
        if (str1->data[i] > str2->data[i]) {
            return 1;
        }
        if (str1->data[i] < str2->data[i]) {
            return -1;
        }
    }
    return 0;
}

int equality(String *str1, String *str2) {
    if (!str1 && !str2) {
        return 1;
    }
    if (!str1 || !str2) {
        return 0;
    }
    int i;
    for (i = 0; str1->data[i] && str2->data[i]; i++) {
        if (str1->data[i] != str2->data[i]) {
            return 0;
        }
    }
    if (str1->data[i] || str2->data[i]) {
        return 0;
    }
    return 1;
}

int copy(String *dest, String *src) {
    if (!dest || !src){
        return 1;
    }
    if (!dest->data){
        return copy_to_new(dest, src);
    }
    for (int i = 0; i <= src->length; i++) {
        if (save_add_char(&(dest->data), &(dest->size_data), i, src->data[i])){
            return 1;
        }
    }
    dest->length = src->length;
    return 0;
}

int copy_to_new(String *dest, String *src) {
    if (!dest || !src){
        return 1;
    }
    if (dest->data != NULL) {
        free(dest->data);
        dest->length = 0;
    }
    dest->size_data = src->length + 1;
    dest->data = (char *) calloc(sizeof(char), (dest->size_data));
    if (!dest->data) {
        return 1;
    }
    for (int i = 0; i <= src->length; i++) {
        dest->data[i] = src->data[i];
    }
    dest->length = src->length;
    return 0;
}

void delete(String *str) {
    if (!str) {
        return;
    }
    if (str->data) {
        free(str->data);
    }
    free(str);
}

int concatenation(String *to, String *from) {
    if (!to || !from) {
        return 1;
    }
    if (from->length == 0){
        return 0;
    }
    if (to->size_data < (to->length + from->length + 1)) {
        to->size_data = to->length + from->length + 1;
        char *temp = (char *) realloc(to->data, sizeof(char) * to->size_data);
        if (!temp) {
            return 1;
        }
        to->data = temp;
    }
    for (int i = 0; i < from->length; i++) {
        to->data[i + to->length] = from->data[i];
    }
    to->data[to->length + from->length] = '\0';
    to->length += from->length;
    return 0;
}

int add_number(String *str, int to_add) {
    char *abc = "0123456789";
    char num[15];
    int number = to_add;
    int i = 0;
    while (number >= 10){
        num[i++] = abc[number % 10];
        number /= 10;
    }
    num[i] = abc[number];
    for (int j = i; 0 <= j; --j){
        if (add(str, num[j])){
            return 1;
        }
    }
    return 0;

}

int add_chars(String *str, char *to_add) {
    for (int i = 0; to_add[i]; i++){
        if (add(str, to_add[i])){
            return 1;
        }
    }
    return 0;
}

int concatenation_char(String *to, char *from) {
    if (!to || !from) {
        return 1;
    }
    if (from[0] == '\0') {
        return 0;
    }
    String *temp_from = create(from);
    if (temp_from == NULL) {
        return 1;
    }
    if (concatenation(to, temp_from)) {
        delete(temp_from);
        return 1;
    }
    delete(temp_from);
    return 0;
}

int read_string(FILE *file, String *str, int *is_end){
    if (str == NULL){
        *is_end = 0;
        return 0;
    }
    str->length = 0;
    int f = 1;
    char c;

    while ((c = (char) fgetc(file)) != EOF){
        if (f){
            if (c == ' ' || c == '\n' || c == '\t' || c == '\r'){ continue; }
            f = 0;
        }
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r'){
            *is_end = 0;
            return str->length;
        }
        else {
            if (add(str, c)){
                *is_end = 0;
                return -1;
            }
        }
    }
    *is_end = 1;
    return str->length;
}