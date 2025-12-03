#ifndef LIB_H_INCLUDED
#define LIB_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

/* ========================================================================= */
/*                          ENUMERATIONS & CONSTANTS                         */
/* ========================================================================= */

typedef enum {
    TOK_EOF,        // Конец входного потока
    TOK_NUMBER,     // Число
    TOK_IDENT,      // Идентификатор (переменная)
    TOK_FUNCTION,   // Функция
    TOK_VECTOR,     // Вектор
    TOK_STRING,     // Строковый литерал (если понадобится)

    // Операторы
    TOK_PLUS,       // +
    TOK_MINUS,      // -
    TOK_UMINUS,     // Унарный минус (u-)
    TOK_MULTIPLY,   // *
    TOK_DIVIDE,     // /
    TOK_ASSIGN,     // =

    // Скобки и разделители
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_LBRACKET,   // [
    TOK_RBRACKET,   // ]
    TOK_COMMA       // ,
} TokenT;

typedef enum {
    CT_INT,
    CT_FLOAT,
    CT_VECTOR,
    CT_STRING
} ContainerType;

/* ========================================================================= */
/*                         FORWARD DECLARATIONS                              */
/* ========================================================================= */

typedef struct Token Token;
typedef struct Ident Ident;
typedef struct Container Container;

/* ========================================================================= */
/*                          DATA STRUCTURES                                  */
/* ========================================================================= */

// --- Data Containers ---

typedef struct {
    int value;
} IntContainer;

typedef struct {
    double value;
} FloatContainer;

typedef struct {
    double x;
    double y;
    double z;
} VectorContainer;

typedef struct {
    char *value;
    size_t length;
} StringContainer;

struct Container {
    ContainerType type;
    void *data;
    void (*free_func)(void*);
    void (*print_func)(void*);
};

// --- Token System ---
struct Token {
    TokenT type;
    char *value;            // Строковое представление (для лексера)
    Container *container;   // Хранение значения (число, вектор и т.д.)
    Token *prev;
    Token *next;
};

// --- Symbol Table (Variables) ---
struct Ident {
    char *name;
    Token *value;
    Ident *prev;
    Ident *next;
};

// --- Function Registry ---

// Указатель на математическую функцию: принимает массив контейнеров и их кол-во
typedef Container* (*MathFunction)(Container* args[], int count);

typedef struct {
    const char* name;
    int arg_count;
    MathFunction func;
} FunctionDef;

/* ========================================================================= */
/*                        CONTAINER API                                      */
/* ========================================================================= */

// Создание
Container* create_int_container(int value);
Container* create_float_container(double value);
Container* create_string_container(const char *value);
Container* create_vector_container(double x, double y, double z);

// Управление памятью
void free_container(Container *container);
void free_int_container(void *data);
void free_float_container(void *data);
void free_string_container(void *data);
void free_vector_container(void *data);

// Операции
Container* get_container(Token* token);
Container* container_deep_copy(Container *src);
int        container_compare(Container *a, Container *b);
double     container_to_double(Container* container);

// Вывод
void print_container(Container *container);
void print_int_container(void *data);
void print_float_container(void *data);
void print_string_container(void *data);
void print_vector_container(void *data);

/* ========================================================================= */
/*                          TOKEN API                                        */
/* ========================================================================= */

// Создание токенов
Token* create_token(TokenT type, const char *value);
Token* create_token_with_container(TokenT type, const char *value, Container *container);
Token* create_number_token(const char *value);
Token* create_string_token(TokenT type, const char *value);
Token* create_vector_token(double x, double y, double z);
Token* copy_token(const Token *src);

// Работа со списками токенов
void add_token(Token **head, Token **tail, Token *token);
void token_set_container(Token *token, Container *container);
void free_token(Token *token);
void free_tokens(Token *head);
void print_token(const Token *token);

// Структуры данных (Стек и Очередь для Shunting Yard)
void   push_to_stack(Token** stack_top, Token* item);
Token* pop_from_stack(Token** stack_top);
void   enqueue(Token** queue_front, Token** queue_rear, Token* item);
Token* dequeue(Token** queue_front, Token** queue_rear);

/* ========================================================================= */
/*                    PARSING & EVALUATION (CORE)                            */
/* ========================================================================= */

// Лексер: превращает строку в список токенов
Token* lex(const char *input);

// Парсер: алгоритм сортировочной станции (преобразует инфиксную запись в RPN)
Token* shuntingYard(Token* tokens);

// Вычислитель: считает результат выражения в обратной польской записи
Container* countRPN(Token *head);

// Главная функция обработки строки
void process_expression(char* input);

/* ========================================================================= */
/*                      SYMBOL TABLE (IDENTIFIERS)                           */
/* ========================================================================= */

Ident* create_ident(char *name, Token *value);
void   add_ident(Ident **first, Ident *new_ident);
void   remove_ident(Ident **first, Ident *ident_to_remove);
Ident* find_ident(Ident *first, const char *name);
void   cleanup_global_data(Ident* FirstIdent);

/* ========================================================================= */
/*                      MATH IMPLEMENTATIONS                                 */
/* ========================================================================= */

// Арифметика
Container* add_func(Container** args, int arg_count);
Container* sub_func(Container** args, int arg_count);
Container* mul_func(Container** args, int arg_count);
Container* div_func(Container** args, int arg_count);
Container* neg_func(Container** args, int arg_count); // Унарный минус

// Математические функции
Container* sin_func(Container** args, int arg_count);
Container* cos_func(Container** args, int arg_count);
Container* log_func(Container** args, int arg_count);
Container* pow_func(Container** args, int arg_count);
Container* abs_func(Container** args, int arg_count);
Container* max_func(Container** args, int arg_count);

// Векторные операции
Container* cross_func(Container** args, int arg_count);

/* ========================================================================= */
/*                        UTILS & FILE I/O                                   */
/* ========================================================================= */

// Логирование
void print_log(const char* format, ...);

// Работа с файлами
void execute_from_file(const char* filename);
void clear_file(const char* filename);
void copy_file(const char* src_name, const char* dst_name);
void append_to_file(const char* filename, const char* text);

#endif // LIB_H_INCLUDED
