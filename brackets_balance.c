/*

ИЗ1 Тимошенко Владимир АПО-12

Составить программу построчной обработки текста. Суть обработки - отбор строк,
содержащих одинаковое количество открывающих и закрывающих круглых скобок.

Программа считывает входные данные со стандартного ввода, и печатает результат в
стандартный вывод.

Процедура отбора нужных строк должна быть оформлена в виде отдельной функции,
которой на вход подается массив строк (который необходимо обработать),
количество переданных строк, а также указатель на переменную,
в которой необходимо разместить результат - массив отобранных строк.
В качестве возвращаемого значения функция должна возвращать количество строк,
содержащихся в результирующем массиве.

Программа должна уметь обрабатывать ошибки - такие как неверные
входные данные(отсутствие входных строк) или ошибки выделения памяти и т.п.
В случае возникновения ошибки нужно выводить об этом сообщение
"[error]" и завершать выполнение программы. 
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 64
#define DEFAULT_ARR_SIZE 3
#define DEFAULT_MULT 2
#define NONE -1


_Bool check_brackets_balance_in_string(char const *const str);
size_t count_strings(char const *const *const string_array, const size_t size,
                     char ***result_array);
void reallocation(char *** arr, size_t *string_array_size, _Bool *allocation_error);
char **read_array_from_input(FILE *stream, size_t *size);


int main() {
  size_t size = 0;

  char **string_array = read_array_from_input(stdin, &size);
  if (!string_array) {
    printf("[error]");
    return 0;
  }

  char **result_array = NULL;

  size_t new_size = count_strings((char const *const *const)string_array, size,
                                  &result_array);

  if (!result_array) {
    printf("[error]");
    for (size_t i = 0; i < size; ++i) {
      free(string_array[i]);
    }
    free(string_array);
    return 0;
  }

  for (size_t i = 0; i < new_size; ++i) {
    printf("%s\n", result_array[i]);
  }

  for (size_t i = 0; i < size; ++i) {
    free(string_array[i]);
  }

  for (size_t i = 0; i < new_size; ++i) {
    free(result_array[i]);
  }

  free(string_array);
  free(result_array);

  return 0;
}


_Bool check_brackets_balance_in_string(char const *const str) {
  if (str == NULL)
    return false;
  int brackets_sum = 0;
  int i = 0;
  while (str[i] != '\0') {

    if (str[i] == ')') {
      brackets_sum -= 1;
    }

    if (str[i] == '(') {
      brackets_sum += 1;
    }
    ++i;
  }

  return brackets_sum ? false : true;
}


size_t count_strings(char const *const *const string_array, const size_t size,
                     char ***result_array) {
  if (!string_array) {
    return 0;
  }

  *result_array = (char **)calloc(DEFAULT_ARR_SIZE, sizeof(char *));
  if (!result_array)
    return 0;

  size_t result_array_it = 0;
  size_t result_array_size = DEFAULT_ARR_SIZE;
  _Bool allocation_error = false;

  for (size_t i = 0; i < size; ++i) {
    if (check_brackets_balance_in_string(string_array[i])) {
      if (result_array_it == result_array_size - 1) {
        reallocation(result_array, &result_array_size, &allocation_error);
      }

      (*result_array)[result_array_it] = (char *)malloc((strlen(string_array[i]) + 1));

      if (!(*result_array)[result_array_it]) {
        allocation_error = true;
        break;
      }

      strcpy((*result_array)[result_array_it], string_array[i]);
      ++result_array_it;
    }
  }

  if (allocation_error) {
    for (size_t i = 0; i < result_array_it; ++i) {
      free((*result_array)[i]);
    }
    free(*result_array);
    *result_array = NULL;
  }

  return result_array_it;
}

char **read_array_from_input(FILE *stream, size_t *size) {
  if (!size)
    return NULL;

  size_t string_array_size = DEFAULT_ARR_SIZE;
  char **arr = (char **)malloc(string_array_size * sizeof(char *));

  if (!arr) {
    return NULL;
  }

  char buffer[BUFFERSIZE] = {0};
  int string_array_it = 0;
  int it_previous = NONE;
  size_t new_str_size = 2 * BUFFERSIZE;
  _Bool allocation_error = false;

  while (fgets(buffer, BUFFERSIZE, stream)) {
    if (string_array_it == string_array_size - 1) {
      reallocation(&arr, &string_array_size, &allocation_error);
    }

    if (string_array_it != it_previous) {
      arr[string_array_it] = (char *)calloc(BUFFERSIZE, sizeof(char *));

      if (!arr[string_array_it]) {
        allocation_error = true;
        break;
      }
    }

    if (strlen(buffer) != BUFFERSIZE - 1 || buffer[strlen(buffer) - 1] == '\n') {
      strcat(arr[string_array_it], buffer);
      if (arr[string_array_it][strlen(arr[string_array_it]) - 1] == '\n')
        arr[string_array_it][strlen(arr[string_array_it]) - 1] = '\0';
      new_str_size = 2 * BUFFERSIZE;
      ++string_array_it;
    } else {
      char *p_arr_new = (char *)realloc(arr[string_array_it], new_str_size);

      if (!p_arr_new) {
        allocation_error = true;
        break;
      }

      arr[string_array_it] = p_arr_new;
      strcat(arr[string_array_it], buffer);
      new_str_size += BUFFERSIZE;
      it_previous = string_array_it;
    }
  }

  if (allocation_error) {
    for (size_t i = 0; i < string_array_it; ++i) {
      free(arr[i]);
    }
    free(arr);
    
    return NULL;
  }

  *size = string_array_it;
  return arr;
}

void reallocation(char ***arr, size_t *string_array_size, _Bool *allocation_error) {
  *string_array_size *= DEFAULT_MULT;
  char **new_arr =
    (char **)realloc(*arr, *string_array_size * sizeof(char *));

  if (!new_arr) {
    *allocation_error = true;
  }

  *arr = new_arr;
}