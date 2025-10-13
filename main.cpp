#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Типы токенов
typedef enum {
    TOK_EOF,        // Конец входного потока
    TOK_NUMBER,     // Число
    TOK_IDENT,      // Идентификатор (переменная)
    TOK_PLUS,       // Оператор +
    TOK_MINUS,      // Оператор -
    TOK_MULTIPLY,   // Оператор *
    TOK_DIVIDE,     // Оператор /
    TOK_LPAREN,     // Левая круглая скобка
    TOK_RPAREN,     // Правая круглая скобка
    TOK_ASSIGN      // Оператор присваивания =
} TokenType;

// Структура токена (узел двусвязного списка)
typedef struct Token {
    TokenType type;
    char *value;
    struct Token *prev;
    struct Token *next;
} Token;


/*
template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* top_node;
    size_t stack_size;

public:
    // Конструктор
    Stack() : top_node(nullptr), stack_size(0) {}

    // Деструктор
    ~Stack() {
        while (!empty()) {
            pop();
        }
    }

    // Добавление элемента на верх стека
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = top_node;
        top_node = new_node;
        stack_size++;
    }

    // Удаление верхнего элемента
    void pop() {
        if (!empty()) {

        Node* temp = top_node;
        top_node = top_node->next;
        delete temp;
        stack_size--;

        }
    }

    // Возврат верхнего элемента
    T& top() {
        if (empty()) {

        }
        else
        {
            return top_node->data;
        }
    }

    // Проверка на пустоту
    bool empty() const {
        return top_node == nullptr;
    }

    // Размер стека
    size_t size() const {
        return stack_size;
    }
};
*/

void push_to_stack(Token* stack_top, Token* item)
{
    if (stack_top == NULL || item == NULL) return;

    // Вставляем элемент перед текущей вершиной стека
    item->prev = stack_top->prev;
    item->next = stack_top;

    if (stack_top->prev != NULL) {
        stack_top->prev->next = item;
    }
    stack_top->prev = item;
}

int pop_from_stack(Token* stack_top, Token* item)
{
    if (stack_top == NULL || item == NULL || stack_top->prev == NULL) {
        return -1; // Ошибка: пустой стек или неверные параметры
    }

    // Извлекаем элемент перед вершиной стека (последний добавленный)
    Token* popped = stack_top->prev;

    // Копируем данные извлеченного элемента
    item->type = popped->type;
    item->value = popped->value;

    // Удаляем извлеченный элемент из списка
    stack_top->prev = popped->prev;
    if (popped->prev != NULL) {
        popped->prev->next = stack_top;
    }

    // Очищаем связи извлеченного элемента
    popped->prev = NULL;
    popped->next = NULL;

    return 0; // Успех
}

void insert_to_queue(Token* queue_end, Token* item)
{
    if (queue_end == NULL || item == NULL) return;

    // Вставляем элемент после текущего конца очереди
    item->prev = queue_end;
    item->next = queue_end->next;

    if (queue_end->next != NULL) {
        queue_end->next->prev = item;
    }
    queue_end->next = item;
}

int get_from_queue(Token* queue_end, Token* item)
{
    if (queue_end == NULL || item == NULL) {
        return -1; // Ошибка: неверные параметры
    }

    // Находим начало очереди (ищем элемент с prev == NULL)
    Token* current = queue_end;
    while (current->prev != NULL) {
        current = current->prev;
    }

    // Если current все еще указывает на queue_end, значит очередь пуста
    if (current == queue_end) {
        return -1; // Очередь пуста
    }

    // Копируем данные первого элемента
    item->type = current->type;
    item->value = current->value;

    // Удаляем первый элемент из очереди
    if (current->next != NULL) {
        current->next->prev = NULL;
    }

    // Очищаем связи извлеченного элемента
    current->next = NULL;
    current->prev = NULL;

    return 0; // Успех
}



// Глобальные переменные для позиции в исходном коде
static const char *src;
static int pos = 0;

// Функция для создания нового токена
Token *create_token(TokenType type, const char *value) {
    Token *token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    token->prev = NULL;
    token->next = NULL;
    return token;
}

// Функция для добавления токена в конец списка
void add_token(Token **head, Token **tail, Token *token) {
    if (*head == NULL) {
        *head = token;
        *tail = token;
    } else {
        (*tail)->next = token;
        token->prev = *tail;
        *tail = token;
    }
}

// Функция для освобождения памяти списка токенов
void free_tokens(Token *head) {
    Token *current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}

// Функция для пропуска пробельных символов
void skip_whitespace() {
    while (isspace(src[pos])) {
        pos++;
    }
}

// Функция для чтения числа
char *read_number() {
    int start = pos;
    while (isdigit(src[pos]) || src[pos] == '.') {
        pos++;
    }
    int length = pos - start;
    char *number = (char*)malloc(length + 1);
    strncpy(number, &src[start], length);
    number[length] = '\0';
    return number;
}

// Функция для чтения идентификатора
char *read_identifier() {
    int start = pos;
    while (isalnum(src[pos]) || src[pos] == '_') {
        pos++;
    }
    int length = pos - start;
    char *ident = (char*)malloc(length + 1);
    strncpy(ident, &src[start], length);
    ident[length] = '\0';
    return ident;
}

// Основная функция лексического анализа
Token *lex(const char *input) {
    src = input;
    pos = 0;

    Token *head = NULL;
    Token *tail = NULL;

    while (src[pos] != '\0') {
        skip_whitespace();

        char current = src[pos];

        if (current == '\0') break;

        // Числа
        if (isdigit(current)) {
            char *number = read_number();
            Token *token = create_token(TOK_NUMBER, number);
            add_token(&head, &tail, token);
            free(number);
            continue;
        }

        // Идентификаторы
        if (isalpha(current) || current == '_') {
            char *ident = read_identifier();
            Token *token = create_token(TOK_IDENT, ident);
            add_token(&head, &tail, token);
            free(ident);
            continue;
        }

        // Операторы и скобки
        Token *token = NULL;
        switch (current) {
            case '+': token = create_token(TOK_PLUS, "+"); break;
            case '-': token = create_token(TOK_MINUS, "-"); break;
            case '*': token = create_token(TOK_MULTIPLY, "*"); break;
            case '/': token = create_token(TOK_DIVIDE, "/"); break;
            case '(': token = create_token(TOK_LPAREN, "("); break;
            case ')': token = create_token(TOK_RPAREN, ")"); break;
            case '=': token = create_token(TOK_ASSIGN, "="); break;
            default:
                printf("Неизвестный символ: %c\n", current);
                pos++;
                continue;
        }

        add_token(&head, &tail, token);
        pos++;
    }

    // Добавляем токен конца файла
    Token *eof = create_token(TOK_EOF, "EOF");
    add_token(&head, &tail, eof);

    return head;
}

void shuntingYard(Token *head, Token *outQueue)
{
    Token* queue_end;
    Token* stack_top;

    Token *current = head;
    /*
    TOK_EOF,        // Конец входного потока
    TOK_NUMBER,     // Число
    TOK_IDENT,      // Идентификатор (переменная)
    TOK_PLUS,       // Оператор +
    TOK_MINUS,      // Оператор -
    TOK_MULTIPLY,   // Оператор *
    TOK_DIVIDE,     // Оператор /
    TOK_LPAREN,     // Левая круглая скобка
    TOK_RPAREN,     // Правая круглая скобка
    TOK_ASSIGN
    */

    while(current != NULL)
    {
        switch(current->type)
        {
        //case Token::INT_LITERAL:
        case TOK_NUMBER:
            insert_to_queue(queue_end,current);
            break;
        case TOK_LPAREN:
        case Token::FUNCTION:
            stack.push(token);
            break;
        }

        current = current->next;
    }

    while (current != NULL) {
        printf("  [%s: %s]\n", type_names[current->type], current->value);
        current = current->prev;
    }
}


// Функция для печати списка токенов
void print_tokens(Token *head) {
    const char *type_names[] = {
        "EOF", "NUMBER", "IDENT", "PLUS", "MINUS",
        "MULTIPLY", "DIVIDE", "LPAREN", "RPAREN", "ASSIGN"
    };

    Token *current = head;
    printf("Токены:\n");
    while (current != NULL) {
        printf("  [%s: %s]\n", type_names[current->type], current->value);
        current = current->next;
    }
}

// Функция для печати токенов в обратном порядке
void print_tokens_reverse(Token *tail) {
    const char *type_names[] = {
        "EOF", "NUMBER", "IDENT", "PLUS", "MINUS",
        "MULTIPLY", "DIVIDE", "LPAREN", "RPAREN", "ASSIGN"
    };

    Token *current = tail;
    printf("Токены (в обратном порядке):\n");
    while (current != NULL) {
        printf("  [%s: %s]\n", type_names[current->type], current->value);
        current = current->prev;
    }
}

// Пример использования
int main() {
    const char *input = "x! = 42 + (y * 3.14) - variable_name";

    Token *tokens = lex(input);

    print_tokens(tokens);


    free_tokens(tokens);

    return 0;
}
