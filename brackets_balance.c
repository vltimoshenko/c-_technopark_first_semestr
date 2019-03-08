/*

ИЗ1 Тимошенко Владимир АПО-12
Run ID: 590

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


int check_string(char const *const str);
size_t count_strings(char const *const *const string_array, const size_t size,
                     char ***result_array);
char **read_array_from_stdin(size_t *size);


int main() {
  size_t size = 0;

  char **string_array = read_array_from_stdin(&size);
  if (!string_array) {
    printf("[Error]");
    return 0;
  }

  char **result_array;

  size_t new_size = count_strings((char const *const *const)string_array, size,
                                  &result_array);

  if (!result_array) {
    printf("[Error]");
    for (size_t i = 0; i < size; ++i)
      free(string_array[i]);
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


int check_string(char const *const str) {
  if (str == NULL)
    return 0;
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

  size_t j = 0;
  size_t result_array_size = DEFAULT_ARR_SIZE;
  int allocation_error = false;

  for (size_t i = 0; i < size; ++i) {
    if (j == result_array_size - 1) {
      char **new_result_array = (char **)realloc(
          *result_array, result_array_size * 2 * sizeof(char *));

      if (!new_result_array) {
        allocation_error = true;
        break;
      }

      *result_array = new_result_array;
      result_array_size *= 2;
    }

    if (check_string(string_array[i])) {
      (*result_array)[j] = (char *)malloc((strlen(string_array[i]) + 1));

      if (!(*result_array)[j]) {
        allocation_error = true;
        break;
      }

      strcpy((*result_array)[j], string_array[i]);
      ++j;
    }
  }

  if (allocation_error) {
    for (size_t k = 0; k < j; ++k)
      free((*result_array)[k]);
    free(*result_array);
    *result_array = NULL;
  }

  return j;
}

char **read_array_from_stdin(size_t *size) {
  if (!size)
    return NULL;

  int string_array_size = DEFAULT_ARR_SIZE;
  char **arr = (char **)malloc(string_array_size * sizeof(char *));

  if (!arr) {
    return NULL;
  }

  char buffer[BUFFERSIZE];
  int i = 0;
  int i_previous = NONE;
  int multiplicator = DEFAULT_MULT;
  int allocation_error = false;

  while (fgets(buffer, BUFFERSIZE, stdin) && buffer[0] != '\n') {
    if (i == string_array_size - 1) {
      string_array_size *= 2;
      char **new_arr =
          (char **)realloc(arr, sizeof(char *) * string_array_size);

      if (!new_arr) {
        allocation_error = true;
        break;
      }

      arr = new_arr;
    }

    if (i != i_previous) {
      arr[i] = (char *)calloc(BUFFERSIZE, sizeof(char *));

      if (!arr[i]) {
        allocation_error = true;
        break;
      }
    }

    if (strlen(buffer) != BUFFERSIZE - 1) {
      strcat(arr[i], buffer);
      if (arr[i][strlen(arr[i]) - 1] == '\n')
        arr[i][strlen(arr[i]) - 1] = '\0';
      multiplicator = DEFAULT_MULT; //значение множителя возвращается к 2 во избежание выделения
      ++i;                          //строк чрезвычайно большого размера(выделяем линейно,если надо)
    } else {
      char *p_arr_new = (char *)realloc(arr[i], BUFFERSIZE * multiplicator);

      if (!p_arr_new) {
        allocation_error = true;
        break;
      }

      arr[i] = p_arr_new;
      strcat(arr[i], buffer);
      ++multiplicator; //мультипликатор увеличивается линейно,
      i_previous = i;  //так как буфер остается прежнего размера
    }
  }

  if (allocation_error) {
    for (size_t j = 0; j < i; ++j)
      free(arr[j]);
    free(arr);
    return NULL;
  }

  *size = i;
  return arr;
}