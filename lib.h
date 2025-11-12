#ifndef LIB_H_INCLUDED
#define LIB_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum {
    TOK_EOF,        // Конец входного потока
    TOK_NUMBER,     // Число
    TOK_IDENT,      // Идентификатор (переменная)
    TOK_PLUS,       // Оператор +
    TOK_MINUS,      // Оператор -
    TOK_UMINUS,     // Унарный минус
    TOK_MULTIPLY,   // Оператор *
    TOK_DIVIDE,     // Оператор /
    TOK_LPAREN,     // Левая круглая скобка
    TOK_RPAREN,     // Правая круглая скобка
    TOK_LBRACKET,   // [
    TOK_RBRACKET,   // ]
    TOK_COMMA,      // ,
    TOK_FUNCTION,   // функция
    TOK_ASSIGN,     // Оператор присваивания =
    TOK_VECTOR         // вектор
} TokenT;


typedef enum
{
    CT_VECTOR,
    CT_FLOAT,
    CT_INT,
    CT_STRING
} ContainerType;

// Типы токенов


typedef struct Container {
    ContainerType type;
    void *data;
    void (*free_func)(void*);     // Функция освобождения данных
    void (*print_func)(void*);    // Функция печати (опционально)
} Container;

typedef struct {
    char *value;
    size_t length;
} StringContainer;

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

// Структура токена (узел двусвязного списка)
typedef struct Token {
    TokenT type;
    char *value;
    Container *container;
    struct Token *prev;
    struct Token *next;
} Token;


typedef struct Ident
{
    char *name;
    Token *value;
    Ident *prev;
    Ident *next;
};

typedef Container* (*MathFunction)(Container* args[], int count);

typedef struct {
    const char* name;
    int arg_count;
    MathFunction func;
} FunctionDef;

Ident* create_ident(char *name, Token *value);
void add_ident(Ident **first, Ident *new_ident);
void remove_ident(Ident **first, Ident *ident_to_remove);
Ident* find_ident(Ident *first, const char *name);

void free_int_container(void *data);
void free_float_container(void *data);
void free_string_container(void *data);
void free_vector_container(void *data);

void print_int_container(void *data);
void print_float_container(void *data);
void print_string_container(void *data);
void print_vector_container(void *data);

Container* create_int_container(int value);
Container* create_float_container(double value);
Container* create_string_container(const char *value);
Container* create_vector_container(double x, double y, double z);
Container* get_container(Token* token);

void free_container(Container *container);
void print_container(Container *container);
Container* container_deep_copy(Container *src);
int container_compare(Container *a, Container *b);
double container_to_double(Container* container);

void add_token(Token **head, Token **tail, Token *token);
void free_token(Token *token);
void free_tokens(Token *head);
void token_set_container(Token *token, Container *container);

Token *create_token_with_container(TokenT type, const char *value, Container *container);
Token *create_token(TokenT type, const char *value);
Token *create_number_token(const char *value);
Token *create_string_token(TokenT type, const char *value);
Token *create_vector_token(double x, double y, double z);
Token *copy_token(const Token *src);
void print_token(const Token *token);

void push_to_stack(Token** stack_top, Token* item);
Token* pop_from_stack(Token** stack_top);

void enqueue(Token** queue_front, Token** queue_rear, Token* item);
Token* dequeue(Token** queue_front, Token** queue_rear) ;


Container* sin_func(Container** args, int arg_count);
Container* cos_func(Container** args, int arg_count);
Container* log_func(Container** args, int arg_count);
Container* pow_func(Container** args, int arg_count);
Container* max_func(Container** args, int arg_count);
Container* abs_func(Container** args, int arg_count);
Container* cross_func(Container** args, int arg_count);
Container* sub_func(Container** args, int arg_count);
Container* add_func(Container** args, int arg_count);
Container* neg_func(Container** args, int arg_count);
Container* div_func(Container** args, int arg_count);
Container* mul_func(Container** args, int arg_count);
#endif // LIB_H_INCLUDED
