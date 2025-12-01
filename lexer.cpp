#include "lib.h"

static const char *src;
static int pos = 0;



void skip_whitespace() {
    while (src[pos] != '\0' && isspace((unsigned char)src[pos])) {
        pos++;
    }
}


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


Token *lex(const char *input) {
    src = input;
    pos = 0;

    Token *head = nullptr;
    Token *tail = nullptr;
    Token *last_token = nullptr;

    while (src[pos] != '\0') {
        skip_whitespace();

        char current = src[pos];

        if (current == '\0') break;


        if (isdigit((unsigned char)current)) {
            char *number = read_number();
            Token *token = create_number_token(number);
            add_token(&head, &tail, token);
            last_token = token;
            free(number);
            continue;
        }


        if (isalpha((unsigned char)current) || current == '_') {
            char *ident = read_identifier();


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


        Token *token = nullptr;
        switch (current) {
            case '+': token = create_token(TOK_PLUS, "+"); break;
            case '-':

                if (is_unary_minus(last_token)) {
                    token = create_token(TOK_UMINUS, "u-");
                } else {
                    token = create_token(TOK_MINUS, "-");
                }
                break;
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
                pos++;
                continue;
        }

        add_token(&head, &tail, token);
        last_token = token;
        pos++;
    }


    Token *eof = create_token(TOK_EOF, "EOF");
    add_token(&head, &tail, eof);

    return head;
}


// Эта функция принимает строку, считает её и пишет результат через print_log
void process_expression(char* input) {
    // 1. Лексический анализ
    Token *tokens = lex(input);
    if (tokens == NULL) {
        print_log("Ошибка лексического анализа\n\n");
        return;
    }

    // 2. Сортировочная станция
    Token* rpn = shuntingYard(tokens);
    if (rpn != NULL) {
        // 3. Вычисление
        Container* result = countRPN(rpn);

        print_log("<< ");

        // ВАЖНО: print_container должен использовать print_log внутри себя!
        // Если вы еще не переделали print_container, раскомментируйте строки ниже:
        /*
        print_container(result); // вывод на экран
        char temp_buf[256];
        // Тут сложная логика записи контейнера в файл,
        // лучше просто замените printf на print_log внутри print_container.
        */
        print_container(result);

        print_log("\n\n");

        free_container(result);
        free_tokens(rpn);
    } else {
        // Ошибки синтаксиса (shuntingYard сам должен писать ошибки через print_log)
        print_log("Ошибка синтаксического анализа\n\n");
    }

    free_tokens(tokens);
}
