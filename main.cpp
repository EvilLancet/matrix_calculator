#include "lib.h"




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

            return container_deep_copy(existing->value->container);
        } else {
            return NULL;
        }
    }
    else
    {
        Container* container = token->container;
        token->container = nullptr;
        return container;
    }
}




int get_priority(TokenT type) {
    switch (type) {
        case TOK_PLUS:
        case TOK_MINUS:
            return 1;
        case TOK_MULTIPLY:
        case TOK_DIVIDE:
            return 2;
        case TOK_UMINUS:
            return 3;
        case TOK_ASSIGN:
            return 0;
        default:
            return -1;
    }
}

int process_comma(Token** stack_top, Token** output_front, Token** output_rear) {


    while (*stack_top) {
        Token* top = *stack_top;


        if (top->type == TOK_LPAREN || top->type == TOK_LBRACKET) {
            break;
        }


        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }


    if (!*stack_top ||
        ((*stack_top)->type != TOK_LPAREN && (*stack_top)->type != TOK_LBRACKET)) {
        printf("Ошибка: запятая находится вне скобок\n");
        return false;
    }

    return true;
}

int process_parenthesis(Token** stack_top, Token** output_front, Token** output_rear) {

    while (*stack_top && (*stack_top)->type != TOK_LPAREN) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }


    if (!*stack_top) {
        printf("Ошибка: несогласованные круглые скобки\n");
        return false;
    }


    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);


    if (*stack_top && (*stack_top)->type == TOK_FUNCTION) {
        Token* func = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, func);
    }

    return true;
}

int process_vector_end(Token** stack_top, Token** output_front, Token** output_rear) {

    while (*stack_top && (*stack_top)->type != TOK_LBRACKET) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }


    if (!*stack_top) {
        printf("Ошибка: несогласованные квадратные скобки\n");
        return false;
    }


    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);



    Token* vector_op = create_token(TOK_VECTOR, "VECTOR");
    enqueue(output_front, output_rear, vector_op);
    return true;
}
Token* shuntingYard(Token* tokens) {
    Token* output_front = NULL;
    Token* output_rear = NULL;
    Token* stack_top = NULL;

    // Флаг состояния: 1 - ждем операнд (число/func/(), 0 - ждем оператор
    int expect_operand = 1;

    Token* current = tokens;
    while (current && current->type != TOK_EOF) {
        switch (current->type) {
            case TOK_NUMBER:
            case TOK_IDENT:
                // ПРОВЕРКА 1: Два числа подряд [2 1] или (5) 5
                if (!expect_operand) {
                    printf("Ошибка: Ожидался оператор или запятая, а встречено число/переменная '%s'\n", current->value);
                    return NULL; // Прерываем выполнение
                }

                enqueue(&output_front, &output_rear, copy_token(current));
                expect_operand = 0; // Теперь ждем оператор
                break;

            case TOK_FUNCTION:
                // Функция может быть только там, где ожидается операнд (как число)
                if (!expect_operand) {
                    printf("Ошибка: Ожидался оператор, встречена функция '%s'\n", current->value);
                    return NULL;
                }
                push_to_stack(&stack_top, copy_token(current));
                // expect_operand остается 1, так как после имени функции обязательно идет '('
                break;

            case TOK_COMMA:
                // Запятая может быть только после операнда (expect_operand == 0)
                if (expect_operand) {
                    printf("Ошибка: Неожиданная запятая (пустой аргумент?)\n");
                    return NULL;
                }
                if(!process_comma(&stack_top, &output_front, &output_rear))return NULL;
                expect_operand = 1; // После запятой ждем следующий аргумент
                break;

            case TOK_LBRACKET:
            case TOK_LPAREN:
                // Открывающая скобка возможна:
                // 1. В начале выражения/аргумента (expect_operand == 1)
                // 2. После функции (expect_operand == 1, но тут тонкий момент, считаем что ок)
                // Если expect_operand == 0, значит это "5 (2)", что является ошибкой (если нет неявного умножения)
                if (!expect_operand) {
                    printf("Ошибка: Ожидался оператор перед скобкой\n");
                    return NULL;
                }

                push_to_stack(&stack_top, copy_token(current));
                expect_operand = 1; // Внутри скобок ждем новое выражение (число)
                break;

            case TOK_RBRACKET:
                // Закрывать скобку можно только после полного выражения (числа)
                if (expect_operand) {
                    printf("Ошибка: Ожидалось значение перед ']'\n");
                    return NULL;
                }
                if(!process_vector_end(&stack_top, &output_front, &output_rear))return NULL;
                expect_operand = 0; // Весь вектор [..] - это операнд, дальше ждем оператор
                break;

            case TOK_RPAREN:
                // Закрывать скобку можно только после полного выражения
                // Исключение: пустые скобки "()". Проверяем, что в стеке '('.
                if (expect_operand) {
                     if (stack_top && stack_top->type == TOK_LPAREN) {
                         // Это пустые скобки, допустимо для функций без аргументов
                     } else {
                        printf("Ошибка: Ожидалось значение перед ')'\n");
                        return NULL;
                     }
                }
                if(!process_parenthesis(&stack_top, &output_front, &output_rear))return NULL;
                expect_operand = 0; // Результат (...) - это операнд, дальше ждем оператор
                break;

            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_MULTIPLY:
            case TOK_DIVIDE:
            case TOK_UMINUS:
            case TOK_ASSIGN:
                {
                    // === УНАРНЫЙ МИНУС ===
                    if (current->type == TOK_MINUS && expect_operand) {
                        push_to_stack(&stack_top, create_token(TOK_UMINUS, "u-"));
                        // expect_operand остается 1, ждем число
                    }
                    else {
                        // === БИНАРНЫЕ ОПЕРАТОРЫ ===
                        // Если ждали число, а пришел "+", значит "5 + * 2" или начало с "+" -> Ошибка
                        if (expect_operand) {
                            printf("Ошибка: Неожиданный оператор '%s' (нет левого операнда)\n", current->value);
                            return NULL;
                        }

                        int current_priority = get_priority(current->type);
                        int is_right_assoc = (current->type == TOK_ASSIGN);

                        while (stack_top != nullptr &&
                               stack_top->type != TOK_LPAREN &&
                               stack_top->type != TOK_LBRACKET) {

                            int top_priority = get_priority(stack_top->type);

                            if ((!is_right_assoc && top_priority >= current_priority) ||
                                (is_right_assoc && top_priority > current_priority)) {

                                Token* op = pop_from_stack(&stack_top);
                                enqueue(&output_front, &output_rear, op);
                            } else {
                                break;
                            }
                        }

                        push_to_stack(&stack_top, create_token(current->type, current->value));
                        expect_operand = 1; // После оператора обязательно ждем операнд
                    }
                }
                break;

            default:
                break;
        }
        current = current->next;
    }

    // В конце строки мы не должны ждать операнда (нельзя заканчивать на "+")
    if (expect_operand) {
        printf("Ошибка: Выражение закончилось неожиданно (ожидался операнд)\n");
        return NULL;
    }

    while (stack_top) {
        Token* op = pop_from_stack(&stack_top);
        if (op->type == TOK_LPAREN || op->type == TOK_LBRACKET) {
            printf("Ошибка: несогласованные скобки (осталась открывающая)\n");
            free_token(op);
            return NULL;
        } else {
            enqueue(&output_front, &output_rear, op);
        }
    }

    return output_front;
}

Token* shuntingYard2(Token* tokens) {
    Token* output_front = NULL;
    Token* output_rear = NULL;
    Token* stack_top = NULL;

    Token* current = tokens;
    while (current && current->type != TOK_EOF) {
        switch (current->type) {
            case TOK_NUMBER:
            case TOK_IDENT:

                enqueue(&output_front, &output_rear, copy_token(current));
                break;

            case TOK_FUNCTION:

                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_COMMA:

                process_comma(&stack_top, &output_front, &output_rear);
                break;

            case TOK_LBRACKET:

                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_LPAREN:

                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_RBRACKET:

                process_vector_end(&stack_top, &output_front, &output_rear);
                break;

            case TOK_RPAREN:

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

                break;
        }
        current = current->next;
    }


    while (stack_top) {
        Token* op = pop_from_stack(&stack_top);


        if (op->type == TOK_LPAREN || op->type == TOK_LBRACKET) {
            printf("Ошибка: несогласованные скобки\n");
            free_token(op);
        } else {
            enqueue(&output_front, &output_rear, op);
        }
    }

    return output_front;
}



Container* container_vector(Container* a, Container* b, Container* c) {
    if (!a || !b || !c) return NULL;


    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT) &&
        (c->type == CT_INT || c->type == CT_FLOAT)) {
        return create_vector_container(container_to_double(a), container_to_double(b), container_to_double(c));
    }
    printf("Ошибка: несовместимые типы для преобразования в вектор\n");
    return NULL;
}


Container* countRPN(Token *head)
{
    Token* stack_top = NULL;
    Token* current = head;

    while (current != NULL) {

        switch (current->type) {
            case TOK_VECTOR:{
                Container** args = extract_args_safely(&stack_top, 3, current->value);
                if (!args) return NULL;

                Container* result = container_vector(args[0], args[1], args[2]);
                Token* result_token = create_token_with_container(TOK_NUMBER, NULL, result);
                push_to_stack(&stack_top, result_token);


                for(int i(0); i<3; i++)
                {
                    if(args[i]) free_container(args[i]);
                }
                free(args);

                break;
            }
            case TOK_NUMBER:
            case TOK_IDENT:


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

            for(int i(0); i<func_def->arg_count; i++)
            {
                if(args[i]) free_container(args[i]);
            }
            free(args);

            if (!result) {
                printf("Ошибка в функции %s\n", current->value);
                return NULL;
            }

            push_to_stack(&stack_top, create_token_with_container(TOK_NUMBER, NULL, result));
            break;
            }

            case TOK_ASSIGN: {

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
                if(value->type == TOK_IDENT)
                {
                    Ident* value_ident = find_ident(FirstIdent, value->value);
                    if(!value_ident)
                    {
                        printf("Ошибка: переменная %s не существует\n", value->value);
                        free_token(ident);
                        free_token(value);
                        return NULL;
                    }
                    free_token(value);
                    value = copy_token(value_ident->value);

                }


                Ident* existing = find_ident(FirstIdent, ident->value);
                if (existing) {

                    free_token(existing->value);
                    existing->value = copy_token(value);
                } else {

                    Ident* new_ident = create_ident(ident->value, copy_token(value));
                    add_ident(&FirstIdent, new_ident);
                }


                push_to_stack(&stack_top, value);
                free_token(ident);

                break;
            }

            default:
                printf("Неизвестный токен в RPN: %d\n", current->type);
                break;
        }
        current = current->next;
    }


    if (!stack_top) {
        printf("Ошибка: пустой стек\n");
        return NULL;
    }

    if (stack_top->next) {
        printf("Ошибка: в стеке осталось несколько элементов\n");

        while (stack_top) {
            Token* temp = pop_from_stack(&stack_top);
            free_token(temp);
        }
        return NULL;
    }


    Token* result_token = pop_from_stack(&stack_top);

    Container* result = NULL;
    if (result_token) {
        result = get_container(result_token);
    }
    free_token(result_token);

    return result;
}


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


void print_help(FILE* screenshot_file) {
    const char* help_text =
        "================ СПРАВКА ПО КАЛЬКУЛЯТОРУ ================\n"
        "\n"
        "КАК ПОЛЬЗОВАТЬСЯ:\n"
        "  Просто введите выражение и нажмите Enter.\n"
        "  Примеры: 2 + 2 * 2, sin(3.14), a = 10\n"
        "\n"
        "УПРАВЛЕНИЕ ПРОГРАММОЙ:\n"
        "  save   - сохранить всю историю введенных команд в файл 'program.txt'\n"
        "  screen - сохранить текущий вид консоли в файл 'screenshot.txt'\n"
        "  open   - загрузить и выполнить команды из файла 'program.txt'\n"
        "  cls    - очистить экран\n"
        "  exit   - закрыть калькулятор\n"
        "  help   - показать этот текст\n"
        "\n"
        "МАТЕМАТИКА И ПЕРЕМЕННЫЕ:\n"
        "  +, -, *, /  : Стандартные операции (сложение, вычитание, ...)\n"
        "  =           : Запомнить число (пример: x = 5 + 2, теперь x равно 7)\n"
        "  [a, b, c]   : Создать вектор из трех чисел (пример: v = [1, 2, 3])\n"
        "\n"
        "ФУНКЦИИ:\n"
        "  sin(x), cos(x) : Синус и косинус (аргумент в радианах)\n"
        "  log(x)         : Натуральный логарифм\n"
        "  abs(x)         : Модуль числа (абсолютное значение)\n"
        "  pow(x, y)      : Возведение x в степень y (вместо x^y)\n"
        "  max(x, y)      : Выбор большего из двух чисел\n"
        "  cross(a, b)    : Векторное произведение двух векторов a и b\n"
        "\n"
        "ПРИМЕРЫ ВЫРАЖЕНИЙ:\n"
        "  >> 5 * (2 + 3)\n"
        "  >> pi = 3.14159\n"
        "  >> sin(pi / 2)\n"
        "  >> vec1 = [1, 0, 0]\n"
        "  >> cross(vec1, [0, 1, 0])\n"
        "=========================================================\n";

    printf("%s", help_text);
    fprintf(screenshot_file, "%s", help_text);
}


void print_welcome_message()
{
    const char* welcome_msg =
        "==========================================================\n"
        "   Консольный Калькулятор (Числа, Векторы, Переменные)    \n"
        "==========================================================\n"
        "Введите математическое выражение и нажмите Enter.\n"
        "Примеры:  2 + 2 * 2  |  x = sin(0.5)  |  v = [1, 2, 3]\n"
        "\n"
        "Введите 'help' для полного списка команд и примеров.\n"
        "Введите 'exit' для выхода.\n"
        "----------------------------------------------------------\n";

    print_log("%s", welcome_msg);
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    clear_file("session.tmp");
    clear_file("history.tmp");

    print_welcome_message(); // Ваша функция с print_log

    char input[256];

    while (1) {
        print_log(">> ");

        if (fgets(input, sizeof(input), stdin) == NULL) break;

        // Логируем ввод пользователя
        append_to_file("session.tmp", input);

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        // Обработка команд
        if (strcmp(input, "exit") == 0) break;

        if (strcmp(input, "save") == 0) {
            copy_file("history.tmp", "program.txt");
            continue;
        }

        if (strcmp(input, "screen") == 0) {
            copy_file("session.tmp", "screenshot.txt");
            continue;
        }

        if (strcmp(input, "cls") == 0) {
            system("cls");
            clear_file("session.tmp");
            continue;
        }

        if (strcmp(input, "help") == 0) {
            print_help(NULL); // print_help должна использовать print_log
            continue;
        }

        // Обработка "open имя_файла"
        if (strncmp(input, "open", 4) == 0) {
            // Если пользователь ввел просто "open", спрашиваем имя (как у вас было)
            // Или парсим имя из строки "open file.txt"
            char filename[256] = "program.txt"; // Дефолтное имя

            // Простая логика: если есть пробел, берем то, что после него
            char* space = strchr(input, ' ');
            if (space != NULL && strlen(space + 1) > 0) {
                strcpy(filename, space + 1);
            } else {
                 // Если преподаватель хочет ввод имени отдельно:
                 print_log("Введите имя файла: ");
                 // Тут нужен fgets, но чтобы не ломать поток print_log...
                 // Проще оставить дефолт program.txt или парсить аргумент
            }

            execute_from_file(filename);
            continue;
        }

        // Если это не команда - это математика

        // 1. Сохраняем в историю
        char cmd_to_save[300];
        sprintf(cmd_to_save, "%s\n", input);
        append_to_file("history.tmp", cmd_to_save);

        // 2. Считаем
        process_expression(input);
    }

    // Очистка
    remove("session.tmp");
    remove("history.tmp");
    cleanup_global_data(FirstIdent);

    return 0;
}
