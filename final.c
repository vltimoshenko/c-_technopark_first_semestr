/*
Требуется написать программу, которая способна вычислять логическе выражения. 
Допустимые выражения чувствительны к регистру и могут содержать: 
1) знаки операций 'and' - и, 'or' - или, 'not' - отрицание, 'xor' - сложение по модулю 2 
2) Скобки '(', ')' 
3) Логические значения 'True', 'False' 
4) Пустое выражение имеет значение "False" 

Также требуется реализовать возможность задания переменных, которые могут состоять
только из маленьких букв латинского алфавита (к примеру, 'x=True'). 
Объявление переменной имеет формат: 
<имя>=<True|False>; // допустимы пробелы 

Допускается несколько объявлений одной и той же переменной, учитывается последнее. 
Все переменные, используемые в выражении, должны быть заданы заранее. Также запрещены имена,
совпадающие с ключевыми словами (and, or и т.д.). 

Необходимо учитывать приоритеты операций. Пробелы ничего не значат. 


Если выражение или объявление не удовлетворяют описанным правилам, требуется вывести в
стандартный поток вывода "[error]" (без кавычек). 

ВАЖНО! Программа в любом случае должна возвращать 0. Не пишите return -1, exit(1) и т.п.
Даже если обнаружилась какая-то ошибка, все равно необходимо вернуть 0! (и напечатать
[error] в stdout).

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFERSIZE 64
#define DEFAULT_ARR_SIZE 3
#define DEFAULT_MULT 2
#define RESULT_STRING_SIZE 1024
#define DEFAULT_SUBSTRING_SIZE 8
#define DEFAULT_STACK_SIZE 4

#define NONE -1
#define FALSE 0
#define TRUE 1

#define ERROR -404
#define MULTIPLIER 2

#define FALSE_STR "False"
#define TRUE_STR "True"
#define NOT "not"
#define XOR "xor"
#define AND "and"
#define OR "or"

#define NOT_PRIORITY 1
#define XOR_PRIORITY 2
#define AND_PRIORITY 3
#define OR_PRIORITY 4
#define BRACKET_PRIORITY 5

#define OPEN_BRACKET "("
#define SPACE " "

typedef struct variable {
    char *name;
    _Bool value;
} variable;

typedef struct stack_tag {
    char **data;
    size_t size;
    size_t top;
} stack_t;

char *strdup(const char *str);                          //those declarations were required by
char *strndup(const char *str, size_t n);               //test-system compiler

char **read_array_from_input(FILE *stream, size_t *size);
void string_array_realloc(char *** arr, size_t *string_array_size, _Bool *allocation_error);

int is_allowed_name(const char *name, const size_t str_size);
int is_variable_in_array(const variable *variable_array, const char *name, const size_t current_size);
char *strip_string(const char *string);
void found_close_bracket(char **result_string, stack_t *stack, char *substring,
                        size_t *substring_it, int *error_flag);
void parse_substring(char *substring, const size_t substring_it, 
                     stack_t *stack, char **result_string, int *flag_error);
void add_to_substring(const char *expression_string,  const size_t expression_str_it,
                      char **substring, size_t *substring_it, size_t *substring_size, int *error_flag);                     
       
stack_t* create_stack();
void delete_stack(stack_t **stack);
stack_t *resize(stack_t *stack);
int push(stack_t *stack, const char *value);
int is_empty(const stack_t *stack);
char* pop(stack_t *stack);
char* get_top(const stack_t *stack);
int check_stack_operators(const stack_t *stack, const char* operator);

int calculate_binary_op(const char* substring, stack_t *stack );
int find_variable_in_arr(const char* substring, const variable *var_array, 
                       const size_t var_arr_size, stack_t *stack);

variable *create_variables_array(const char **string_array, const size_t *size, size_t *var_array_size);
char **string_conversion_to_RPN(const char* expression_string, char **result_string);
int calculate_expression(const char* expression, const size_t var_arr_size,
                           variable *var_array);

int main() {
    size_t size = 0;
    char **string_array = read_array_from_input(stdin, &size);
    if (!string_array) {
        printf("[error]");
    }
    
    size_t var_arr_size = 0;
    variable *var_array = NULL;
    if (size > 1) {
      var_array = create_variables_array((const char**)string_array, &size, &var_arr_size);
      if (!var_arr_size || !var_array) {
        printf("[error]");
        for (size_t i = 0; i < size; ++i) {
          free(string_array[i]);
        }
        free(string_array);
        return 0;
      }
    }

    char *result_string = NULL;
    result_string = *string_conversion_to_RPN(string_array[size - 1], &result_string);
    if (!result_string) {
      printf("[error]");
    }

    if (result_string) {
      int result = calculate_expression(result_string, var_arr_size, var_array);
    
      if (result == ERROR) printf("[error]");
      else printf("%s", result == TRUE ? "True" : "False");

      free(result_string);
    }

    for (size_t i = 0; i < size; ++i) {
        free(string_array[i]);
    }

    for (size_t i = 0; i < size - 1; ++i) {
        free(var_array[i].name);
    }

    free(string_array);
    free(var_array);
    return 0;
}

char **read_array_from_input(FILE *stream, size_t *size) {
  if (!size || !stream) {
    return NULL;
  }

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
      string_array_realloc(&arr, &string_array_size, &allocation_error);
      if (allocation_error) break;
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

void string_array_realloc(char ***arr, size_t *string_array_size, _Bool *allocation_error) {
  if (!arr || !allocation_error || !string_array_size) {
    *allocation_error = true;
    return;
  }

  *string_array_size *= DEFAULT_MULT;
  char **new_arr =
    (char **)realloc(*arr, *string_array_size * sizeof(char *));

  if (!new_arr) {
    *allocation_error = true;
  }

  *arr = new_arr;
}

int is_allowed_name(const char *name, const size_t str_size) {
  if (!name) return ERROR;
  if (!str_size) return FALSE;
  if (strcmp(TRUE_STR, name) == 0) return FALSE;
  if (strcmp(FALSE_STR, name) == 0) return FALSE;

  if (strcmp(AND, name) == 0) return FALSE;
  if (strcmp(OR, name) == 0) return FALSE;
  if (strcmp(XOR, name) == 0) return FALSE;
  if (strcmp(NOT, name) == 0) return FALSE;

  int error_flag = FALSE;
  for (size_t i = 0; i < strlen(name); ++i) {
    if (!islower(name[i])) {
      error_flag = TRUE;
      break;
    }
  }
  if (error_flag) return FALSE;
  else return TRUE;
}

variable *create_variables_array(const char **string_array, const size_t *size, size_t *var_array_size) {
    if (!string_array || !var_array_size || !size) return NULL;
    if (*size <= 1) return NULL;

    *var_array_size = *size - 1;
    variable *variable_array = (variable *)malloc((*var_array_size) * sizeof(variable));
    if (!variable_array) return NULL;

    size_t current_length = 0;
    _Bool error_flag = false;
    char *name = NULL;
    char *value = NULL;

    for (size_t i = 0; i < *var_array_size; ++i) {
        size_t length = strlen(string_array[i]);

        for (size_t j = 0; j < length; ++j) {
            if (string_array[i][j] == '=') {
                name = (char *)malloc(j + 1);
                if (!name) {
                  error_flag = true;
                  break;
                }

                strncpy(name, string_array[i], j);
                name[j] = '\0';

                char *striped_name = strip_string(name);
                if (!striped_name) {
                  error_flag = true;
                  break;
                }

                free(name);
                name = striped_name;
                striped_name = NULL;

                if (!is_allowed_name(name, strlen(name))) {
                  error_flag = true;
                  break;
                };

                int index_to_write = is_variable_in_array(variable_array, name, current_length);
                if (index_to_write == ERROR) {
                  error_flag = true;
                  break;
                } 

                if (index_to_write == i) ++current_length;
                variable_array[index_to_write].name = name;
                name = NULL;

                value = (char *)malloc(length - j - 1);
                if (!value) {
                  error_flag = true;
                  break;
                }

                strncpy(value, string_array[i] + j + 1, length - j - 2);
                value[length - j - 2] = '\0';
                
                char *striped_value = strip_string(value);
                if (!striped_value) {
                  error_flag = true;
                  break;
                }

                free(value);
                value = striped_value;
                striped_value = NULL;

                if (strcmp(value, FALSE_STR) == 0) variable_array[index_to_write].value = false;
                else if (strcmp(value, TRUE_STR) == 0) variable_array[index_to_write].value = true;
                else {
                  error_flag = true;
                  break;
                }; 

                free(value);
                value = NULL;
                break;
            }
        }
    }
    if (error_flag) {
      if (name) free(name);
      if (value) free(name);

      for (size_t i = 0; i < current_length; ++i) {
        free(variable_array[i].name);
      }
      free(variable_array);
      
      variable_array = NULL;
      var_array_size = NULL;
    } else {
      *var_array_size = current_length;
    }
    return variable_array;
}

int is_variable_in_array(const variable *variable_array, const char *name, const size_t current_size) {
    if (!variable_array || !name) return ERROR;

    _Bool found = false;
    size_t i = 0;
    for (; i < current_size; ++i) {
        if (strcmp(variable_array[i].name, name) == 0) break;
    }
    if (found) return i;
    return current_size;
}

char *strip_string(const char *string) {
    if (!string) return NULL;
    size_t length = strlen(string);

    while (isspace(string[length - 1]))
        --length;
    while (*string && isspace(*string))
        ++string, --length;

    char *res = strndup(string, length);
    if (!res) return NULL;
    return res;
}

 
void found_close_bracket(char **result_string, stack_t *stack, char *substring, 
                         size_t *substring_it, int *error_flag) {
  if (!result_string || !stack || !substring ||
      !substring_it || !error_flag) {
        *error_flag = true;
        return;
  }

  substring[*substring_it] = '\0';
  strcat(*result_string, " ");
  strcat(*result_string, substring);
  char *string_from_get_top = NULL;
  char *string_from_pop = NULL;

  while (true) {
    string_from_get_top = get_top(stack);
    if (!string_from_get_top) {
      *error_flag = TRUE;
      break;
    }
    if (strcmp(OPEN_BRACKET, string_from_get_top) == 0) break;

    strcat(*result_string, " ");

    string_from_pop = pop(stack);
    if (!string_from_pop) {
      *error_flag = TRUE;
      break;  //or no need?
    }
    strcat(*result_string, string_from_pop);

    free(string_from_get_top);
    free(string_from_pop);
  }
  free(string_from_get_top); //or check

  string_from_pop = pop(stack);
  if (!string_from_pop) *error_flag = TRUE;

  free(string_from_pop); //
  substring_it = 0;
}

char **string_conversion_to_RPN(const char* expression_string, char **result_string) {
  stack_t *stack = create_stack();
  if (!stack) return NULL;

  *result_string = (char*)calloc(RESULT_STRING_SIZE, sizeof(char));
  if (!result_string) {
    delete_stack(&stack);
    return NULL;
  }

  size_t substring_size = DEFAULT_SUBSTRING_SIZE;
  char *substring = (char*)calloc(substring_size, sizeof(char));

  size_t substring_it = 0;
  int error_flag = FALSE;

  for (size_t i = 0; i < strlen(expression_string); ++i) {
    
    if (isalpha(expression_string[i])) {
      add_to_substring(expression_string, i, &substring, 
                       &substring_it, &substring_size, &error_flag);
      if (error_flag) break;
    }

    if (expression_string[i] == ')') {
      found_close_bracket(result_string, stack,  substring, &substring_it, &error_flag);
      if (error_flag) break;
      continue;
    }

    if (i > 0) {
      if (((expression_string[i] == ' ' ||  expression_string[i] == '(') &&
           isalpha(expression_string[i - 1])) || i == strlen(expression_string) - 1) {
        parse_substring(substring, substring_it, stack, result_string, &error_flag);
        if (error_flag) break; 
      }
    }

    if (expression_string[i] == ' ') {
      substring_it = 0;
    } else if (expression_string[i] == '(') {
      if (push(stack, "(") == ERROR) {
        error_flag = TRUE;
        break;
      }
    }  
  }

  while (!is_empty(stack)) {
    strcat(*result_string, " ");
    char *string_from_pop = pop(stack);
    if (!string_from_pop) {
      error_flag = TRUE;
      break;
    }
    strcat(*result_string, string_from_pop);
    free(string_from_pop);
  }

  free(substring);
  delete_stack(&stack);

  if (error_flag) {
    free(result_string);
    return NULL;
  }

  return result_string;
}

int calculate_expression(const char* expression_string, const size_t var_arr_size,
                           variable *var_array) {
  if (!expression_string || (var_arr_size > 0 && !var_array)) {
    return ERROR;
  }

  stack_t *stack = create_stack();
  if (!stack) return ERROR;

  size_t substring_size = DEFAULT_SUBSTRING_SIZE;
  char *substring = (char*)calloc(substring_size, sizeof(char));
  if (!substring) return ERROR;

  int error_flag = false;
  size_t substring_it = 0;
  for (size_t i = 0; i < strlen(expression_string); ++i) {
    
    if (isalpha(expression_string[i])) {
      add_to_substring(expression_string, i, &substring, 
                       &substring_it, &substring_size, &error_flag);
      if (error_flag) break;
    }

    if (i > 0) {
      if ((expression_string[i] == ' ' && isalpha(expression_string[i - 1])) || 
                                         i == strlen(expression_string) - 1) {
        substring[substring_it] = '\0';

        if (strcmp(substring, NOT) == 0) {
          char *string1_from_stack = pop(stack);
          if (!string1_from_stack) {
            error_flag = TRUE;
            break;
          }

          if (strcmp(string1_from_stack, TRUE_STR) == 0) {
            if (push(stack, FALSE_STR) == ERROR) error_flag = TRUE;
          }

          if (strcmp(string1_from_stack, FALSE_STR) == 0) {
            if (push(stack, TRUE_STR) == ERROR) error_flag = TRUE;
          }

          free(string1_from_stack);
          if (error_flag == TRUE) break;
        } else if (strcmp(substring, AND) == 0 || strcmp(substring, XOR) == 0 
                                         || strcmp(substring, OR) == 0) {
          if (calculate_binary_op(substring, stack) == ERROR) {
            error_flag = TRUE;
            break;
          }
        } else if (strcmp(substring, TRUE_STR) == 0) {
          if (push(stack, TRUE_STR) == ERROR) {
            error_flag = TRUE;
            break;
          }
        } else if (strcmp(substring, FALSE_STR) == 0) {
          if (push(stack, FALSE_STR) == ERROR){
            error_flag = TRUE;
            break;
          }
        } 
        else {
          if (find_variable_in_arr(substring, var_array, var_arr_size, stack) == ERROR) {
            error_flag = TRUE;
            break;
          }
        }
        substring_it = 0;
      }
    }
  }

  int result = FALSE;
  if (!error_flag) {
    char *result_string = pop(stack);
    if (strcmp(result_string, TRUE_STR) == 0) result = TRUE;
    free(result_string);
  }

  free(substring);
  delete_stack(&stack);
  
  if (error_flag) return ERROR;
  return result;
}

#define DEFAULT_STACK_SIZE 4

stack_t* create_stack() {
    stack_t *result = NULL;
    result = (stack_t *)malloc(sizeof(stack_t));
    if (!result) {
        return NULL;
    } 
    result->size = DEFAULT_STACK_SIZE;
    result->data = (char**)malloc(result->size * sizeof(char*));
    if (!result->data) {
        free(result);
        return NULL;
    }
    result->top = 0;
    return result;
}

void delete_stack(stack_t **stack) {
    if (!stack) return;
    for (size_t i = 0; i < (*stack)->top; ++i) {
      free((*stack)->data[i]);
    }
    free((*stack)->data);
    free(*stack);
}

stack_t *resize(stack_t *stack) {
    if (!stack) return NULL;

    stack->size *= MULTIPLIER;
    char **new_data = (char **)realloc(stack->data, stack->size * sizeof(char *));
    if (!new_data) {
        return NULL;
    }

    stack->data = new_data;
    return stack;
}

int push(stack_t *stack, const char *value) {
    if (!stack || !value) return ERROR;

    char *new_value = strdup(value);
    if (!new_value) return ERROR;

    if (stack->top >= stack->size) {
        if (!resize(stack)) return ERROR;
    }
    stack->data[stack->top] = new_value;
    stack->top++;
    return 0;
}

int is_empty(const stack_t *stack) {
  if (!stack) return ERROR;
  return stack->top == 0 ? TRUE : FALSE;
}

char* pop(stack_t *stack) {
    if (!stack || stack->top == 0) return NULL;
    
    stack->top--;
    char *result = strdup(stack->data[stack->top]);
    if (!result) return NULL;

    free(stack->data[stack->top]);
    return result;
}

char* get_top(const stack_t *stack) {
    if (!stack || stack->top == 0) return NULL;
    
    char *result = strdup(stack->data[stack->top - 1]);
    return result;
}

int check_stack_operators(const stack_t *stack, const char* operator) {
  if (!stack || !operator) return ERROR;

  if (is_empty(stack)) return FALSE;
  int priority = NONE;

  if (strcmp(operator, AND) == 0) priority = AND_PRIORITY;
  else if (strcmp(operator, NOT) == 0) priority = NOT_PRIORITY;
  else if (strcmp(operator, XOR) == 0) priority = XOR_PRIORITY;
  else if (strcmp(operator, OR) == 0) priority = OR_PRIORITY;
  else if (strcmp(operator, OPEN_BRACKET) == 0) priority = BRACKET_PRIORITY;

  int stack_priority = NONE;
  char *stack_operator = get_top(stack);
  if (!stack_operator) return ERROR;

  if (strcmp(stack_operator, AND) == 0) stack_priority = AND_PRIORITY;
  else if (strcmp(stack_operator, NOT) == 0) stack_priority = NOT_PRIORITY;
  else if (strcmp(stack_operator, XOR) == 0) stack_priority = XOR_PRIORITY;
  else if (strcmp(stack_operator, OR) == 0) stack_priority = OR_PRIORITY;
  else if (strcmp(stack_operator, OPEN_BRACKET) == 0) stack_priority = BRACKET_PRIORITY;

  free(stack_operator);
  return stack_priority > priority ? FALSE : TRUE;
}

void parse_substring(char *substring, const size_t substring_it, 
                     stack_t *stack, char **result_string, int *error_flag) {
  if (!substring || !stack || !result_string || !error_flag) {
    *error_flag = TRUE;
    return;
  }

  substring[substring_it] = '\0';                         
  if (strcmp(substring, AND) == 0 || strcmp(substring, NOT) == 0 || 
      strcmp(substring, XOR) == 0 || strcmp(substring, OR) == 0 ) {
    
    while (true) {
      int stack_state = check_stack_operators(stack, substring);
      if (stack_state == ERROR) {
        *error_flag = TRUE;
        break;
      }

      if (stack_state == FALSE) break;

      strcat(*result_string, " ");
      char *string_from_pop = pop(stack);
      if (!string_from_pop) {
        *error_flag = TRUE;
        break;
      }

      strcat(*result_string, string_from_pop);
      free(string_from_pop);
    }
    if (*error_flag == TRUE) {
        return;
    }

    char *string_for_stack = (char*)calloc(strlen(substring) + 1, sizeof(char));
    if (!string_for_stack) {
      *error_flag = TRUE;
      return;
    }

    strcpy(string_for_stack, substring);
    if (push(stack, string_for_stack) == ERROR) {
      *error_flag = TRUE;
    }

    free(string_for_stack);
  } else {
    strcat(*result_string, " ");
    strcat(*result_string, substring);
  }
}


void add_to_substring(const char *expression_string,  const size_t expression_str_it,
                      char **substring, size_t *substring_it, size_t *substring_size, int *error_flag) {
  if (!expression_string || !substring
      || !substring_it || !substring_size || !error_flag) {
    *error_flag = TRUE;
    return;
  }
  (*substring)[*substring_it] = expression_string[expression_str_it];
  ++(*substring_it);

  if (*substring_it == *substring_size - 1) {
    char *new_substring = (char*)realloc(*substring, (*substring_size) *= MULTIPLIER);
    if (!new_substring) {
      *error_flag = TRUE;
      return;
    }
    *substring = new_substring;
  }
}

int calculate_binary_op(const char* substring, stack_t *stack ) {
  if (!substring || !stack) return ERROR;
  
  char* string1_from_stack = pop(stack);
  if (!string1_from_stack) return ERROR;

  char* string2_from_stack = pop(stack);
  if (!string2_from_stack) {
    free(string1_from_stack);
    return ERROR;
  }

  _Bool value1 = false;
  if (strcmp(string1_from_stack, TRUE_STR) == 0) value1 = true;

  _Bool value2 = false;
  if (strcmp(string2_from_stack, TRUE_STR) == 0) value2 = true;

  int result = FALSE;
  if (strcmp(substring, AND) == 0) {
    result = value1 && value2;
  } else if (strcmp(substring, XOR) == 0) {
    result = value1 ^ value2;
  } else if (strcmp(substring, OR) == 0) {
    result = value1 || value2;
  }

  if (result) {
    if (push(stack, TRUE_STR) == ERROR) {
      result = ERROR;
    } 
  } else {
    if (push(stack, FALSE_STR) == ERROR) {
      result = ERROR;
    }
  }
  
  free(string1_from_stack);
  free(string2_from_stack);

  return result;
}

int find_variable_in_arr(const char* substring, const variable *var_array, 
                       const size_t var_arr_size, stack_t *stack) {
  if (!substring || !stack || (var_arr_size > 0 && !var_array)) {
    return ERROR;
  }

  _Bool found_flag = false;
  for (size_t k = 0; k < var_arr_size; ++k) {
    if (strcmp(substring, var_array[k].name) == 0) {
      if (var_array[k].value) {
        if (push(stack, TRUE_STR) == ERROR) break;
      } else if (push(stack, FALSE_STR) == ERROR) break;
      found_flag = true;
      break;
    }
  }
  if (!found_flag) {
    return ERROR;
  }
  return TRUE;
}