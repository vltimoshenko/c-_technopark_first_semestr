#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 64

int check_string(char const *const str);
size_t count_strings(char const *const *const string_array, 
                     char** result_array, const size_t size);
char** read_array_from_stdin(size_t *size);


int main() {
    size_t size = 0;

    char **string_array = read_array_from_stdin(&size);
    if (!string_array) {
        printf("[Error]");
        return 0;
    }

    char **result_array = (char**)calloc(size, sizeof(char*));
    if (!result_array) {
        printf("[Error]");
        for (size_t i = 0; i < size; ++i) free(string_array[i]);
        free(string_array);
        return 0;
    }

    size_t new_size = count_strings((char const *const *const) string_array, 
                                    result_array, size);

    for (size_t i = 0; i < new_size; ++i) {
        printf("%s", result_array[i]);
    }

    for (size_t i = 0; i < size; ++i) {
        free(string_array[i]);
        free(result_array[i]);
    }
    free(string_array);
    free(result_array);

    return 0;
}


int check_string(char const *const str) {
    if (str == NULL) return 0;
    int brackets_sum = 0;
    int i = 0;
    while(str[i] != '\0'){
        if (brackets_sum < 0) return 0;
        if (str[i] == ')') {
            brackets_sum -= 1;
        }
        if (str[i] == '(') {
            brackets_sum += 1;
        }
        ++i;
    }

    return brackets_sum ? 0 : 1;
}


size_t count_strings(char const *const *const string_array,
                     char** result_array, const size_t size) {
    if (!string_array || ! result_array) {
        printf("[Error]");
        return 0;
    }

    size_t j = 0;

    for (size_t i = 0; i < size; ++i) {
        if(check_string(string_array[i])) {
            result_array[j] = (char*)malloc((strlen(string_array[i]) + 1) * sizeof(char));
            if (!result_array[j]) {
                printf("[Error]");
                return j;
            }
            strcpy(result_array[j], string_array[i]);
            ++j;
        }
    }
    return j;
}


char** read_array_from_stdin(size_t *size) {
    int string_array_size = 3;
    char** arr = (char**)malloc(string_array_size * sizeof(char*));
    if (!arr) {
        printf("[error]");
        return 0;
    }
    char buffer[BUFFERSIZE];
    int i = 0;
    int i_last = -1;
    int multiplicator = 2;

    while(fgets(buffer, BUFFERSIZE, stdin) && buffer[0] != '\n') {
        if (i == string_array_size - 1) {
            string_array_size *= 2;
            char ** new_arr = (char**)realloc(arr, sizeof(char*) * string_array_size);
            if (!new_arr) {
                printf("[error]");
                for (size_t j = 0; j < i; ++j) free(arr[j]);
                free(arr);
                return 0;
            }
            arr = new_arr;
            new_arr = NULL;
        }

        if (i != i_last) {
            arr[i] = (char*)calloc(BUFFERSIZE, sizeof(char*));
            if (!arr[i]) {
                printf("[error]");
                for (size_t j = 0; j < i; ++j) free(arr[j]);
                free(arr);
                return 0;
            }
        }

        if (strlen(buffer) != BUFFERSIZE - 1) {
            strcat(arr[i], buffer);
            multiplicator = 2;
            ++i;
        } else {
            char *p_arr_new = (char*)realloc(arr[i], BUFFERSIZE * multiplicator);
            if (!p_arr_new) {
                printf("[error]");
                for (size_t j = 0; j < i; ++j) free(arr[j]);
                free(arr);
                return 0;
            }
            arr[i] = p_arr_new;
            p_arr_new = NULL;
            strcat(arr[i], buffer);
            ++multiplicator;
            i_last = i;
        }
    }

    *size = i;
    return arr;
}