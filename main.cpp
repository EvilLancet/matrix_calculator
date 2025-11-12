#include "lib.h"


// Глобальные переменные для позиции в исходном коде

static const char *src;
static int pos = 0;

Ident* FirstIdent;

FunctionDef functions[14] = {
    {"sin",   1, sin_func  },
    {"cos",   1, cos_func  },
    {"log",   1, log_func  },
    {"pow",   2, pow_func  },
    {"max",   2, max_func  },
    {"cross", 2, cross_func},
    {"abs" ,  1, abs_func  },
    {"-",     2, sub_func  },
    {"+",     2, add_func  },
    {"u-",    1, neg_func  },
    {"/",     2, div_func  },
    {"*",     2, mul_func  },
    {NULL,    0, NULL}
};

FunctionDef* find_function(const char* name) {
    for (FunctionDef* f = functions; f->name; f++) {
        if (strcmp(f->name, name) == 0) return f;
    }
    return NULL;
}

int stack_size(Token* stack_top)
{
    int count = 0;
    for (Token* current = stack_top; current != nullptr; current = current->next) {
        count++;
    }
    return count;
}

Container** extract_args_safely(Token** stack, int arg_count, const char* func_name) {
    if (stack_size(*stack) < arg_count) {
        printf("Недостаточно аргументов для %s (нужно %d)\n", func_name, arg_count);
        return NULL;
    }

    Container** args = (Container**)malloc(arg_count * sizeof(Container*));
    if (!args) return NULL;

    for (int i = arg_count - 1; i >= 0; i--) {
        Token* token = pop_from_stack(stack);
        if (!token) {
            // Освобождаем уже извлеченные контейнеры
            for (int j = arg_count - 1; j > i; j--) {
                free_container(args[j]);
            }
            free(args);
            return NULL;
        }
        args[i] = get_container(token);
        free_token(token);
    }

    return args;
}


Container* get_container(Token* token)
{
    if(token->type == TOK_IDENT)
    {
        Ident* existing = find_ident(FirstIdent, token->value);
        if (existing) {
            // Обновляем существующий идентификатор
            return existing->value->container;
        } else {
            return NULL;
        }
    }
    else
    {
        return token->container;
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

// Функция для определения унарного минуса
int is_unary_minus(Token *last_token) {
    // Унарный минус, если:
    // 1. Это первый токен
    // 2. После оператора (+, -, *, /, =)
    // 3. После открывающей скобки
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

// Основная функция лексического анализа
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

        // Числа
        if (isdigit((unsigned char)current)) {
            char *number = read_number();
            Token *token = create_number_token(number);
            add_token(&head, &tail, token);
            last_token = token;
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
            last_token = token;
            free(ident);
            continue;
        }

        // Операторы, скобки и новые символы
        Token *token = nullptr;
        switch (current) {
            case '+': token = create_token(TOK_PLUS, "+"); break;
            case '-':
                // Проверяем, является ли минус унарным
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
        case TOK_UMINUS:
            return 3; // Унарный минус имеет высший приоритет
        case TOK_ASSIGN:
            return 0;
        default:
            return -1;
    }
}

void process_comma(Token** stack_top, Token** output_front, Token** output_rear) {
    // Выталкиваем операторы из стека до тех пор,
    // пока не встретим левую круглую или квадратную скобку
    while (*stack_top) {
        Token* top = *stack_top;

        // Останавливаемся на открывающих скобках
        if (top->type == TOK_LPAREN || top->type == TOK_LBRACKET) {
            break;
        }

        // Выталкиваем оператор в выходную очередь
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

    // Проверяем, что запятая находится внутри скобок
    if (!*stack_top ||
        ((*stack_top)->type != TOK_LPAREN && (*stack_top)->type != TOK_LBRACKET)) {
        printf("Ошибка: запятая находится вне скобок\n");
        return;
    }
}

void process_parenthesis(Token** stack_top, Token** output_front, Token** output_rear) {
    // Выталкиваем все операторы до открывающей круглой скобки
    while (*stack_top && (*stack_top)->type != TOK_LPAREN) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

    // Проверяем, что нашли открывающую скобку
    if (!*stack_top) {
        printf("Ошибка: несогласованные круглые скобки\n");
        return;
    }

    // Удаляем открывающую круглую скобку из стека
    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);

    // Если на вершине стека находится функция, перемещаем ее в выходную очередь
    if (*stack_top && (*stack_top)->type == TOK_FUNCTION) {
        Token* func = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, func);
    }
}

void process_vector_end(Token** stack_top, Token** output_front, Token** output_rear) {
    // Выталкиваем все операторы до открывающей квадратной скобки
    while (*stack_top && (*stack_top)->type != TOK_LBRACKET) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

    // Проверяем, что нашли открывающую скобку
    if (!*stack_top) {
        printf("Ошибка: несогласованные квадратные скобки\n");
        return;
    }

    // Удаляем открывающую квадратную скобку из стека
    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);

    // Добавляем специальный оператор VECTOR в выходную очередь
    // Этот оператор будет обработан при вычислении RPN
    Token* vector_op = create_token(TOK_VECTOR, "VECTOR");
    enqueue(output_front, output_rear, vector_op);
}

Token* shuntingYard(Token* tokens) {
    Token* output_front = NULL;
    Token* output_rear = NULL;
    Token* stack_top = NULL;

    Token* current = tokens;
    while (current && current->type != TOK_EOF) {
        switch (current->type) {
            case TOK_NUMBER:
            case TOK_IDENT:
                // Числа и идентификаторы сразу в выход
                enqueue(&output_front, &output_rear, copy_token(current));
                break;

            case TOK_FUNCTION:
                // Функции помещаем в стек
                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_COMMA:
                // Запятая - разделитель аргументов
                process_comma(&stack_top, &output_front, &output_rear);
                break;

            case TOK_LBRACKET:
                // Открывающая квадратная скобка - в стек
                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_LPAREN:
                // Открывающая круглая скобка - в стек
                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_RBRACKET:
                // Закрывающая квадратная скобка - обрабатываем вектор
                process_vector_end(&stack_top, &output_front, &output_rear);
                break;

            case TOK_RPAREN:
                // Закрывающая круглая скобка
                process_parenthesis(&stack_top, &output_front, &output_rear);
                break;

            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_MULTIPLY:
            case TOK_DIVIDE:
            case TOK_UMINUS:
            case TOK_ASSIGN:
                {
                    int current_priority = get_priority(current->type);

                    while (stack_top != nullptr &&
                           get_priority(stack_top->type) >= current_priority &&
                           stack_top->type != TOK_LPAREN) {
                        Token* op = pop_from_stack(&stack_top);
                        enqueue(&output_front, &output_rear, op);
                    }

                    push_to_stack(&stack_top, create_token(current->type, current->value));
                }
                break;

            default:
                // Пропускаем неизвестные токены
                break;
        }
        current = current->next;
    }

    // После обработки всех токенов выталкиваем оставшиеся операторы из стека
    while (stack_top) {
        Token* op = pop_from_stack(&stack_top);

        // Проверяем, что не осталось несогласованных скобок
        if (op->type == TOK_LPAREN || op->type == TOK_LBRACKET) {
            printf("Ошибка: несогласованные скобки\n");
            free_token(op);
        } else {
            enqueue(&output_front, &output_rear, op);
        }
    }

    return output_front;
}




// Функции арифметических операций
Container* container_vector(Container* a, Container* b, Container* c) {
    if (!a || !b || !c) return NULL;

    // Числа
    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT) &&
        (c->type == CT_INT || c->type == CT_FLOAT)) {
        return create_vector_container(container_to_double(a), container_to_double(b), container_to_double(c));
    }
    printf("Ошибка: несовместимые типы для преобразования в вектор\n");
    return NULL;
}

Container* apply_function(const char* func_name, Container* arg) {
    if (!func_name || !arg) return NULL;

    // Проверяем, что аргумент - число
    if (arg->type != CT_INT && arg->type != CT_FLOAT) {
        printf("Ошибка: функция %s применяется только к числам\n", func_name);
        return NULL;
    }

    double value = container_to_double(arg);
    double result = 0.0;

    if (strcmp(func_name, "sin") == 0) {
        result = sin(value);
    } else if (strcmp(func_name, "cos") == 0) {
        result = cos(value);
    } else if (strcmp(func_name, "tan") == 0) {
        result = tan(value);
    } else if (strcmp(func_name, "sqrt") == 0) {
        if (value < 0) {
            printf("Ошибка: квадратный корень из отрицательного числа\n");
            return NULL;
        }
        result = sqrt(value);
    } else if (strcmp(func_name, "exp") == 0) {
        result = exp(value);
    } else if (strcmp(func_name, "log") == 0) {
        if (value <= 0) {
            printf("Ошибка: логарифм неположительного числа\n");
            return NULL;
        }
        result = log(value);
    } else {
        printf("Неизвестная функция: %s\n", func_name);
        return NULL;
    }

    return create_float_container(result);
}

Container* countRPN(Token *head)
{
    Token* stack_top = NULL;
    Token* current = head;

    while (current != NULL) {

        switch (current->type) {
            case TOK_VECTOR:{
                Token* c = pop_from_stack(&stack_top);
                Token* b = pop_from_stack(&stack_top);
                Token* a = pop_from_stack(&stack_top);
                Container* result = container_vector(get_container(a), get_container(b), get_container(c));
                Token* result_token = create_token_with_container(TOK_NUMBER, NULL, result);
                push_to_stack(&stack_top, result_token);
                free_token(a);
                free_token(b);
                free_token(c);
                break;
            }
            case TOK_NUMBER:
            case TOK_IDENT:

                // Копируем токен и помещаем в стек
                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_MULTIPLY:
            case TOK_DIVIDE:
            case TOK_UMINUS:
            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_FUNCTION: {
            FunctionDef* func_def = find_function(current->value);
            if (!func_def) {
                printf("Неизвестная функция: %s\n", current->value);
                return NULL;
            }

            Container** args = extract_args_safely(&stack_top, func_def->arg_count, current->value);
            if (!args) return NULL;

            Container* result = func_def->func(args, func_def->arg_count);
            free(args);

            if (!result) {
                printf("Ошибка в функции %s\n", current->value);
                return NULL;
            }

            push_to_stack(&stack_top, create_token_with_container(TOK_NUMBER, NULL, result));
            break;
            }

            case TOK_ASSIGN: {
                // Обработка присваивания (a = 5)
                if (!stack_top || !stack_top->next) {
                    printf("Ошибка: недостаточно операндов для =\n");
                    return NULL;
                }
                Token* value = pop_from_stack(&stack_top);
                Token* ident = pop_from_stack(&stack_top);

                if (ident->type != TOK_IDENT) {
                    printf("Ошибка: слева от = должен быть идентификатор\n");
                    free_token(ident);
                    free_token(value);
                    return NULL;
                }

                // Сохраняем значение в таблице идентификаторов
                Ident* existing = find_ident(FirstIdent, ident->value);
                if (existing) {
                    // Обновляем существующий идентификатор
                    free_token(existing->value);
                    existing->value = copy_token(value);
                } else {
                    // Создаем новый идентификатор
                    Ident* new_ident = create_ident(ident->value, copy_token(value));
                    add_ident(&FirstIdent, new_ident);
                }

                // Результат присваивания - значение
                push_to_stack(&stack_top, copy_token(value));
                free_token(ident);
                free_token(value);
                break;
            }

            default:
                printf("Неизвестный токен в RPN: %d\n", current->type);
                break;
        }
        current = current->next;
    }

    // Проверяем, что в стеке остался ровно один элемент
    if (!stack_top) {
        printf("Ошибка: пустой стек\n");
        return NULL;
    }

    if (stack_top->next) {
        printf("Ошибка: в стеке осталось несколько элементов\n");
        // Очищаем стек
        while (stack_top) {
            Token* temp = pop_from_stack(&stack_top);
            free_token(temp);
        }
        return NULL;
    }

    // Извлекаем результат
    Token* result_token = pop_from_stack(&stack_top);

    Container* result = NULL;
    if (result_token) {
        result = container_deep_copy(get_container(result_token));
    }
    free_token(result_token);

    return result;
}

// Функция для печати списка токенов
void print_tokens(Token *head) {
    const char *type_names[] = {
        "EOF", "NUMBER", "IDENT", "PLUS", "MINUS",
        "UMINUS", "MULTIPLY", "DIVIDE", "LPAREN", "RPAREN",
        "LBRACKET", "RBRACKET", "COMMA", "FUNCTION", "ASSIGN"
    };

    Token *current = head;
    printf("Токены:\n");
    while (current != nullptr) {
        printf("  [%s: %s]\n", type_names[current->type], current->value);
        current = current->next;
    }
}

// Функция для печати выражения в ОПЗ
void print_rpn(Token* head) {
    printf("Выражение в ОПЗ: ");
    Token* current = head;
    while (current != nullptr) {
        printf("%s ", current->value);
        current = current->next;
    }
    printf("\n");
}

char *screenshot_bu = "screen.bu";
char *screenshot = "screen.txt";

char *program_bu = "program.bu";
char *program = "program.txt";


void copy_file(const char* source, const char* destination) {
    FILE* src = fopen(source, "r");
    if (src == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", source);
        return;
    }

    FILE* dst = fopen(destination, "w");
    if (dst == NULL) {
        printf("Ошибка: не удалось создать файл %s\n", destination);
        fclose(src);
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    printf("Файл успешно скопирован: %s -> %s\n", source, destination);
}

// Функция для выполнения команд из файла program.txt
void execute_from_program_txt(FILE* program_file, FILE* screenshot_file) {
    FILE* input_file = fopen("program.txt", "r");
    if (input_file == NULL) {
        printf("Ошибка: не удалось открыть файл program.txt\n");
        return;
    }

    char line[256];
    Container* result;

    printf("Выполнение команд из program.txt...\n");
    fprintf(screenshot_file, "Выполнение команд из program.txt...\n");

    while (fgets(line, sizeof(line), input_file) != NULL) {
        // Удаляем символ новой строки
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        // Пропускаем команды save/screen/open при выполнении из файла
        if (strcmp(line, "save") == 0 || strcmp(line, "screen") == 0 ||
            strcmp(line, "open") == 0 || strcmp(line, "exit") == 0) {
            printf(">> %s\n", line);
            fprintf(screenshot_file, ">> %s\n", line);
            printf("<< Команда пропущена при выполнении из файла\n");
            fprintf(screenshot_file, "<< Команда пропущена при выполнении из файла\n");
            continue;
        }

        // Выводим команду в консоль и screenshot
        printf(">> %s\n", line);
        fprintf(screenshot_file, ">> %s\n", line);

        // Обрабатываем команду как математическое выражение
        Token *tokens = lex(line);
        if (tokens == nullptr) {
            printf("Ошибка лексического анализа\n\n");
            fprintf(screenshot_file, "Ошибка лексического анализа\n\n");
            continue;
        }

        Token* rpn = shuntingYard(tokens);
        if (rpn != nullptr) {
            // Вычисление результата
            result = countRPN(rpn);
            printf("<< ");
            fprintf(screenshot_file, "<< ");

            // Вывод результата
            print_container(result);
            printf("\n");
            fprintf(screenshot_file, "\n");

            // Записываем команду в program_bu файл
            fprintf(program_file, "%s\n", line);

            free_tokens(rpn);
        } else {
            printf("Ошибка синтаксического анализа\n\n");
            fprintf(screenshot_file, "Ошибка синтаксического анализа\n\n");
        }

        free_tokens(tokens);
    }

    fclose(input_file);
    printf("Выполнение команд из program.txt завершено.\n");
    fprintf(screenshot_file, "Выполнение команд из program.txt завершено.\n");
}


int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    FILE* screenshot = fopen(screenshot_bu, "w");
    FILE* program = fopen(program_bu, "w");

    char input[256];
    Container* result;

    printf("Калькулятор запущен. Для выхода введите 'exit'.\n");
    printf("Доступные команды:\n");
    printf("  save   - сохранить program в program.txt\n");
    printf("  screen - сохранить screenshot в screenshot.txt\n");
    printf("  open   - выполнить команды из program\n\n");

    // Записываем начальное сообщение в screenshot
    fprintf(screenshot, "Калькулятор запущен. Для выхода введите 'exit'.\n");
    fprintf(screenshot, "Доступные команды:\n");
    fprintf(screenshot, "  save   - сохранить program в program.txt\n");
    fprintf(screenshot, "  screen - сохранить screenshot в screenshot.txt\n");
    fprintf(screenshot, "  open   - выполнить команды из program\n\n");

    while (1) {
        printf(">> ");
        fprintf(screenshot, ">> ");

        // Считываем ввод пользователя
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Записываем ввод в screenshot
        fprintf(screenshot, "%s", input);

        // Удаляем символ новой строки
        input[strcspn(input, "\n")] = 0;

        // Проверяем на выход
        if (strcmp(input, "exit") == 0) {
            printf("Выход из калькулятора.\n");
            fprintf(screenshot, "Выход из калькулятора.\n");
            break;
        }

        // Обработка команды save
        if (strcmp(input, "save") == 0) {
            copy_file(program_bu, "program.txt");
            continue;
        }

        // Обработка команды screen
        if (strcmp(input, "screen") == 0) {
            fflush(screenshot);
            copy_file(screenshot_bu, "screenshot.txt");
            continue;
        }

        // Обработка команды open
        if (strcmp(input, "open") == 0) {
            execute_from_program_txt(program, screenshot);
            continue;
        }

        // Пропускаем пустые строки
        if (strlen(input) == 0) {
            continue;
        }

        // Здесь ваш существующий код обработки выражений
        // Лексический анализ
        Token *tokens = lex(input);
        if (tokens == nullptr) {
            printf("Ошибка лексического анализа\n\n");
            fprintf(screenshot, "Ошибка лексического анализа\n\n");
            continue;
        }

        // Синтаксический анализ и преобразование в ОПН
        Token* rpn = shuntingYard(tokens);
        if (rpn != nullptr) {
            // Вычисление результата
            result = countRPN(rpn);
            printf("<< ");
            fprintf(screenshot, "<< ");

            // Вывод результата
            print_container(result);
            fputc('\n', stdout);
            //print_container(result);
            fputc('\n', screenshot);

            // Записываем входное выражение в program файл
            fprintf(program, "%s\n", input);
            fflush(program);
            free_tokens(rpn);
        } else {
            printf("Ошибка синтаксического анализа\n\n");
            fprintf(screenshot, "Ошибка синтаксического анализа\n\n");
        }

        free_tokens(tokens);
    }

    fclose(screenshot);
    fclose(program);

    return 0;
}
