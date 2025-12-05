#include "lib.h"


// Глобальные переменные состояния лексера
static const char *src;
static int pos = 0;


// Пропуск пробелов и управляющих символов
void skip_whitespace() {
    while (src[pos] != '\0' && isspace((unsigned char)src[pos])) {
        pos++;
    }
}

// Чтение числа из строки
char *read_number() {
    int start = pos;
    // Считываем цифры и точку, если она есть
    while (isdigit((unsigned char)src[pos]) || src[pos] == '.') {
        pos++;
    }
    int length = pos - start;
    char *number = (char*)malloc(length + 1);
    strncpy(number, &src[start], length);
    number[length] = '\0';
    return number;
}

// Чтение идентификатора
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

// Определение, является ли минус унарным
int is_unary_minus(Token *last_token) {

    if (last_token == nullptr) {
        return 1;
    }

    switch (last_token->type) {
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_MULTIPLY:
        case TOK_DIVIDE:
        case TOK_ASSIGN:
        case TOK_LPAREN:
        case TOK_LBRACKET:
        case TOK_COMMA:
            return 1;
        default:
            return 0;
    }
}

//Функция лексического анализа
Token *lex(const char *input) {
    src = input;
    pos = 0;

    Token *head = nullptr;
    Token *tail = nullptr;

    // Последний добавленный токен
    Token *last_token = nullptr;

    while (src[pos] != '\0') {
        skip_whitespace();

        char current = src[pos];

        if (current == '\0') break;

        // Обработка чисел
        if (isdigit((unsigned char)current)) {
            char *number = read_number();
            Token *token = create_number_token(number);
            add_token(&head, &tail, token);
            last_token = token;
            free(number);
            continue;
        }

        // Обработка идентификаторов
        if (isalpha((unsigned char)current) || current == '_') {
            char *ident = read_identifier();

            // Проверка на функцию
            int is_func = 0;
            int next_pos = pos;
            skip_whitespace();
            if (src[pos] == '(') {
                is_func = 1;
            }
            pos = next_pos;

            Token *token = create_token(is_func ? TOK_FUNCTION : TOK_IDENT, ident);
            add_token(&head, &tail, token);
            last_token = token;
            free(ident);
            continue;
        }

        // Обработка операторов и символов пунктуации
        Token *token = nullptr;
        switch (current) {
            case '+': token = create_token(TOK_PLUS, "+"); break;
            case '-': token = create_token(TOK_MINUS, "-"); break;
            case '*': token = create_token(TOK_MULTIPLY, "*"); break;
            case '/': token = create_token(TOK_DIVIDE, "/"); break;
            case '(': token = create_token(TOK_LPAREN, "("); break;
            case ')': token = create_token(TOK_RPAREN, ")"); break;
            case '=': token = create_token(TOK_ASSIGN, "="); break;
            case '[': token = create_token(TOK_LBRACKET, "[");break;
            case ']': token = create_token(TOK_RBRACKET, "]"); break;
            case ',': token = create_token(TOK_COMMA, ","); break;
            default:
                printf("Неизвестный символ: %c\n", current);
                free_tokens(head);
                return NULL;
                //pos++;
                //continue;
        }

        add_token(&head, &tail, token);
        last_token = token;
        pos++;
    }

    // Добавление токена конца ввода
    Token *eof = create_token(TOK_EOF, "EOF");
    add_token(&head, &tail, eof);

    return head;
}


