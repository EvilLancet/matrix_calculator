#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    TOK_LBRACKET,   // [
    TOK_RBRACKET,   // ]
    TOK_COMMA,      // ,
    TOK_FUNCTION,   // функция
    TOK_ASSIGN      // Оператор присваивания =
} TokenT;

// Структура токена (узел двусвязного списка)
typedef struct Token {
    TokenT type;
    char *value;
    struct Token *prev;
    struct Token *next;
} Token;

// Функции для работы со стеком
void push_to_stack(Token** stack_top, Token* item) {
    if (stack_top == NULL || item == NULL) return;

    item->prev = NULL;
    item->next = *stack_top;

    if (*stack_top != NULL) {
        (*stack_top)->prev = item;
    }

    *stack_top = item;
}

Token* pop_from_stack(Token** stack_top) {
    if (stack_top == NULL || *stack_top == NULL) {
        return NULL;
    }

    Token* popped = *stack_top;
    *stack_top = (*stack_top)->next;

    if (*stack_top != NULL) {
        (*stack_top)->prev = NULL;
    }

    popped->next = NULL;
    popped->prev = NULL;
    return popped;
}

// Функции для работы с очередью
void enqueue(Token** queue_front, Token** queue_rear, Token* item) {
    if (item == NULL) return;

    item->next = NULL;
    item->prev = *queue_rear;

    if (*queue_rear != NULL) {
        (*queue_rear)->next = item;
    }

    *queue_rear = item;

    if (*queue_front == NULL) {
        *queue_front = item;
    }
}

Token* dequeue(Token** queue_front, Token** queue_rear) {
    if (queue_front == NULL || *queue_front == NULL) {
        return NULL;
    }

    Token* dequeued = *queue_front;
    *queue_front = (*queue_front)->next;

    if (*queue_front != NULL) {
        (*queue_front)->prev = NULL;
    } else {
        *queue_rear = NULL;
    }

    dequeued->next = NULL;
    dequeued->prev = NULL;
    return dequeued;
}

// Глобальные переменные для позиции в исходном коде
static const char *src;
static int pos = 0;

// Функция для создания нового токена
Token *create_token(TokenT type, const char *value) {
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
    while (src[pos] != '\0' && isspace((unsigned char)src[pos])) {
        pos++;
    }
}

// Функция для чтения числа
char *read_number() {
    int start = pos;
    while (isdigit((unsigned char)src[pos]) || src[pos] == '.') {
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
    while (isalnum((unsigned char)src[pos]) || src[pos] == '_') {
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
        if (isdigit((unsigned char)current)) {
            char *number = read_number();
            Token *token = create_token(TOK_NUMBER, number);
            add_token(&head, &tail, token);
            free(number);
            continue;
        }

        // Идентификаторы и функции
        if (isalpha((unsigned char)current) || current == '_') {
            char *ident = read_identifier();

            // Проверяем, является ли идентификатор функцией
            int is_func = 0;
            int next_pos = pos;
            skip_whitespace();
            if (src[pos] == '(') {
                is_func = 1;
            }
            pos = next_pos; // Возвращаем позицию

            Token *token = create_token(is_func ? TOK_FUNCTION : TOK_IDENT, ident);
            add_token(&head, &tail, token);
            free(ident);
            continue;
        }

        // Операторы, скобки и новые символы
        Token *token = NULL;
        switch (current) {
            case '+': token = create_token(TOK_PLUS, "+"); break;
            case '-': token = create_token(TOK_MINUS, "-"); break;
            case '*': token = create_token(TOK_MULTIPLY, "*"); break;
            case '/': token = create_token(TOK_DIVIDE, "/"); break;
            case '(': token = create_token(TOK_LPAREN, "("); break;
            case ')': token = create_token(TOK_RPAREN, ")"); break;
            case '=': token = create_token(TOK_ASSIGN, "="); break;
            case '[': token = create_token(TOK_LBRACKET, "["); break;
            case ']': token = create_token(TOK_RBRACKET, "]"); break;
            case ',': token = create_token(TOK_COMMA, ","); break;
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

// Функция для получения приоритета оператора
int get_priority(TokenT type) {
    switch (type) {
        case TOK_PLUS:
        case TOK_MINUS:
            return 1;
        case TOK_MULTIPLY:
        case TOK_DIVIDE:
            return 2;
        case TOK_ASSIGN:
            return 0;
        default:
            return -1;
    }
}

// Алгоритм сортировочной станции (Shunting Yard)
Token* shuntingYard(Token *head) {
    if (head == NULL) return NULL;

    Token *output_front = NULL;
    Token *output_rear = NULL;
    Token *stack_top = NULL;

    Token *current = head;

    while (current != NULL && current->type != TOK_EOF) {
        switch (current->type) {
            case TOK_NUMBER:
            case TOK_IDENT:
                enqueue(&output_front, &output_rear, create_token(current->type, current->value));
                break;

            case TOK_FUNCTION:
                push_to_stack(&stack_top, create_token(current->type, current->value));
                break;

            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_MULTIPLY:
            case TOK_DIVIDE:
            case TOK_ASSIGN:
                {
                    int current_priority = get_priority(current->type);

                    while (stack_top != NULL &&
                           get_priority(stack_top->type) >= current_priority) {
                        Token* op = pop_from_stack(&stack_top);
                        enqueue(&output_front, &output_rear, op);
                    }

                    push_to_stack(&stack_top, create_token(current->type, current->value));
                }
                break;

            case TOK_LPAREN:
                push_to_stack(&stack_top, create_token(current->type, current->value));
                break;

            case TOK_RPAREN:
                while (stack_top != NULL && stack_top->type != TOK_LPAREN) {
                    Token* op = pop_from_stack(&stack_top);
                    enqueue(&output_front, &output_rear, op);
                }

                if (stack_top != NULL && stack_top->type == TOK_LPAREN) {
                    Token* temp = pop_from_stack(&stack_top);
                    free(temp->value);
                    free(temp);
                }

                if (stack_top != NULL && stack_top->type == TOK_FUNCTION) {
                    Token* func = pop_from_stack(&stack_top);
                    enqueue(&output_front, &output_rear, func);
                }
                break;

            default:
                // Игнорируем другие токены
                break;
        }

        current = current->next;
    }

    // Перемещаем все оставшиеся операторы из стека в выходную очередь
    while (stack_top != NULL) {
        Token* op = pop_from_stack(&stack_top);
        enqueue(&output_front, &output_rear, op);
    }

    return output_front;
}

double countRPN(Token *head) {
    double stack[100]; // Стек для чисел
    int top = -1;      // Вершина стека

    Token *current = head;
    while (current != NULL) {
        switch (current->type) {
            case TOK_NUMBER: {
                // Преобразуем строку в число и помещаем в стек
                double num = atof(current->value);
                stack[++top] = num;
                break;
            }

            case TOK_PLUS:
                if (top < 1) {
                    printf("Ошибка: недостаточно операндов для +\n");
                    return 0;
                }
                stack[top - 1] = stack[top - 1] + stack[top];
                top--;
                break;

            case TOK_MINUS:
                if (top < 1) {
                    printf("Ошибка: недостаточно операндов для -\n");
                    return 0;
                }
                stack[top - 1] = stack[top - 1] - stack[top];
                top--;
                break;

            case TOK_MULTIPLY:
                if (top < 1) {
                    printf("Ошибка: недостаточно операндов для *\n");
                    return 0;
                }
                stack[top - 1] = stack[top - 1] * stack[top];
                top--;
                break;

            case TOK_DIVIDE:
                if (top < 1) {
                    printf("Ошибка: недостаточно операндов для /\n");
                    return 0;
                }
                if (stack[top] == 0) {
                    printf("Ошибка: деление на ноль\n");
                    return 0;
                }
                stack[top - 1] = stack[top - 1] / stack[top];
                top--;
                break;

            default:
                // Игнорируем другие токены
                break;
        }
        current = current->next;
    }

    if (top != 0) {
        printf("Ошибка: в стеке осталось %d элементов\n", top + 1);
        return 0;
    }

    return stack[0];
}



// Функция для печати списка токенов
void print_tokens(Token *head) {
    const char *type_names[] = {
        "EOF", "NUMBER", "IDENT", "PLUS", "MINUS",
        "MULTIPLY", "DIVIDE", "LPAREN", "RPAREN",
        "LBRACKET", "RBRACKET", "COMMA", "FUNCTION", "ASSIGN"
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
        "MULTIPLY", "DIVIDE", "LPAREN", "RPAREN",
        "LBRACKET", "RBRACKET", "COMMA", "FUNCTION", "ASSIGN"
    };

    Token *current = tail;
    printf("Токены (в обратном порядке):\n");
    while (current != NULL) {
        printf("  [%s: %s]\n", type_names[current->type], current->value);
        current = current->prev;
    }
}

// Функция для печати выражения в ОПЗ
void print_rpn(Token* head) {
    printf("Выражение в ОПЗ: ");
    Token* current = head;
    while (current != NULL) {
        printf("%s ", current->value);
        current = current->next;
    }
    printf("\n");
}

// Пример использования
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    const char *input = "b = a = (42 - 10) * 3 + 10 * 10";
    printf("Входное выражение: %s\n", input);

    Token *tokens = lex(input);
    print_tokens(tokens);

    Token* rpn = shuntingYard(tokens);
    if (rpn != NULL) {
        print_rpn(rpn);
        double a = countRPN(rpn);
        printf("%f - result", a);
        free_tokens(rpn);
    }

    free_tokens(tokens);

    return 0;
}
