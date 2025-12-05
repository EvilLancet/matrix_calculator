#include "lib.h"



// Глобальный список переменных
Ident* FirstIdent;

// Таблица поддерживаемых функций и количество их аргументов
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

// Линейный поиск функции по имени в таблице
FunctionDef* find_function(const char* name) {
    for (FunctionDef* f = functions; f->name; f++) {
        if (strcmp(f->name, name) == 0) return f;
    }
    return NULL;
}

// Подсчет элементов в стеке
int stack_size(Token* stack_top)
{
    int count = 0;
    for (Token* current = stack_top; current != nullptr; current = current->next) {
        count++;
    }
    return count;
}

// Безопасное извлечение N аргументов из стека
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


// Получение контейнера из токена
Container* get_container(Token* token)
{
    if(token->type == TOK_IDENT)
    {
        // Если это переменная, ищем её значение в глобальном списке
        Ident* existing = find_ident(FirstIdent, token->value);
        if (existing) {
            // Делаем копию, чтобы не повредить переменную
            return container_deep_copy(existing->value->container);
        } else {
            print_log("Ошибка: переменная %s не существует\n", token->value);
            return NULL;
        }
    }
    else
    {
        Container* container = token->container;
        // Отвязываем контейнер от токена
        token->container = nullptr;
        return container;
    }
}


// Определение приоритета операторов для сортировочной станции
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

// Обработка запятой: выталкивание операторов до открывающей скобки
int process_comma(Token** stack_top, Token** output_front, Token** output_rear) {


    while (*stack_top) {
        Token* top = *stack_top;

        // Ищем границу текущего аргумента (скобку функции или вектора)
        if (top->type == TOK_LPAREN || top->type == TOK_LBRACKET) {
            break;
        }


        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

     // Если стек кончился, а скобки нет — ошибка парсинга
    if (!*stack_top ||
        ((*stack_top)->type != TOK_LPAREN && (*stack_top)->type != TOK_LBRACKET)) {
        printf("Ошибка: запятая находится вне скобок\n");
        return false;
    }

    return true;
}


// Обработка закрывающей круглой скобки
int process_parenthesis(Token** stack_top, Token** output_front, Token** output_rear) {

    // Выталкиваем всё в выходную очередь до открывающей скобки
    while (*stack_top && (*stack_top)->type != TOK_LPAREN) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }


    if (!*stack_top) {
        printf("Ошибка: несогласованные круглые скобки\n");
        return false;
    }

    // Удаляем открывающую скобку из стека
    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);

    // Если перед скобкой была функция (например, sin(..)), отправляем её в выходную очередь
    if (*stack_top && (*stack_top)->type == TOK_FUNCTION) {
        Token* func = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, func);
    }

    return true;
}

// Обработка закрывающей квадратной скобки
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


    // Генерируем специальный оператор TOK_VECTOR, который скажет вычислителю создать вектор
    Token* vector_op = create_token(TOK_VECTOR, "VECTOR");
    enqueue(output_front, output_rear, vector_op);
    return true;
}

// Алгоритм сортировочной станции
Token* shuntingYard(Token* tokens) {
    Token* output_front = NULL;
    Token* output_rear = NULL;
    Token* stack_top = NULL;

    // Флаг состояния: 1 - ждем операнд, 0 - ждем оператор
    int expect_operand = 1;

    Token* current = tokens;
    while (current && current->type != TOK_EOF) {
        switch (current->type) {
            case TOK_NUMBER:
            case TOK_IDENT:
                // Проверка двух чисел подряд
                if (!expect_operand) {
                    printf("Ошибка: Ожидался оператор или запятая, а встречено число/переменная '%s'\n", current->value);
                    return NULL; // Прерываем выполнение
                }

                enqueue(&output_front, &output_rear, copy_token(current));
                expect_operand = 0; // Теперь ждем оператор
                break;

            case TOK_FUNCTION:
                // Функция может быть только там, где ожидается операнд
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
                 // Открывающая скобка возможна в начале выражения или после оператора/функции
                if (!expect_operand) {
                    printf("Ошибка: Ожидался оператор перед скобкой\n");
                    return NULL;
                }

                push_to_stack(&stack_top, copy_token(current));
                expect_operand = 1; // Внутри скобок ждем новое выражение
                break;

            case TOK_RBRACKET:
                // Закрывать скобку можно только после полного выражения
                if (expect_operand) {
                    printf("Ошибка: Ожидалось значение перед ']'\n");
                    return NULL;
                }
                if(!process_vector_end(&stack_top, &output_front, &output_rear))return NULL;
                expect_operand = 0; // Весь вектор [..] - это операнд, дальше ждем оператор
                break;

            case TOK_RPAREN:
                // Исключение: пустые скобки "()" допустимы только если в стеке '('
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
                    // Если минус встречен там, где ждем число (начало строки или после скобки)
                    if (current->type == TOK_MINUS && expect_operand) {
                        push_to_stack(&stack_top, create_token(TOK_UMINUS, "u-"));
                        // expect_operand остается 1, ждем число
                    }
                    else {

                        if (expect_operand) {
                            printf("Ошибка: Неожиданный оператор '%s' (нет левого операнда)\n", current->value);
                            return NULL;
                        }

                        int current_priority = get_priority(current->type);
                        int is_right_assoc = (current->type == TOK_ASSIGN);

                        // Выталкивание операторов с большим или равным приоритетом
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

    // В конце строки мы не должны ждать операнда
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


// Создание вектора из 3 компонентов
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

// Вычисление выражения в Обратной Польской Записи
Container* countRPN(Token *head)
{
    Token* stack_top = NULL;
    Token* current = head;

    while (current != NULL) {

        switch (current->type) {
            case TOK_VECTOR:{
                // Сборка вектора из 3 чисел на стеке
                Container** args = extract_args_safely(&stack_top, 3, current->value);
                if (!args) return NULL;

                Container* result = container_vector(args[0], args[1], args[2]);
                Token* result_token = create_token_with_container(TOK_NUMBER, NULL, result);
                push_to_stack(&stack_top, result_token);

                // Очистка временных аргументов
                for(int i(0); i<3; i++)
                {
                    if(args[i]) free_container(args[i]);
                }
                free(args);

                break;
            }
            case TOK_NUMBER:
            case TOK_IDENT:

                // Числа и переменные просто кладем в стек
                push_to_stack(&stack_top, copy_token(current));
                break;

            case TOK_MULTIPLY:
            case TOK_DIVIDE:
            case TOK_UMINUS:
            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_FUNCTION: {
            //Поиск функции
            FunctionDef* func_def = find_function(current->value);
            if (!func_def) {
                print_log("Неизвестная функция: %s\n", current->value);
                return NULL;
            }

            Container** args = extract_args_safely(&stack_top, func_def->arg_count, current->value);
            if (!args) return NULL;

            Container* result = func_def->func(args, func_def->arg_count);

            // Освобождение аргументов после вычисления
            for(int i(0); i<func_def->arg_count; i++)
            {
                if(args[i]) free_container(args[i]);
            }
            free(args);

            if (!result) {
                print_log("Ошибка в функции %s\n", current->value);
                return NULL;
            }

            // Результат кладем обратно в стек
            push_to_stack(&stack_top, create_token_with_container(TOK_NUMBER, NULL, result));
            break;
            }

            case TOK_ASSIGN: {

                if (!stack_top || !stack_top->next) {
                    print_log("Ошибка: недостаточно операндов для =\n");
                    return NULL;
                }
                Token* value = pop_from_stack(&stack_top);
                Token* ident = pop_from_stack(&stack_top);

                if (ident->type != TOK_IDENT) {
                    print_log("Ошибка: слева от = должен быть идентификатор\n");
                    free_token(ident);
                    free_token(value);
                    return NULL;
                }
                // Если справа переменная, берем её значение
                if(value->type == TOK_IDENT)
                {
                    Ident* value_ident = find_ident(FirstIdent, value->value);
                    if(!value_ident)
                    {
                        print_log("Ошибка: переменная %s не существует\n", value->value);
                        free_token(ident);
                        free_token(value);
                        return NULL;
                    }
                    free_token(value);
                    value = copy_token(value_ident->value);

                }

                 // Поиск существующей переменной или создание новой
                Ident* existing = find_ident(FirstIdent, ident->value);
                if (existing) {
                    // Обновление значения существующей переменной
                    free_token(existing->value);
                    existing->value = copy_token(value);
                } else {
                    // Создание новой переменной в списке
                    Ident* new_ident = create_ident(ident->value, copy_token(value));
                    add_ident(&FirstIdent, new_ident);
                }

                // Результат присваивания (значение) возвращается в стек
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

    // В конце вычисления в стеке должен остаться ровно один элемент
    if (!stack_top) {
        printf("Ошибка: пустой стек\n");
        return NULL;
    }


    if (stack_top->next) {
        printf("Ошибка: в стеке осталось несколько элементов\n");
        // Очистка мусора при ошибке
        while (stack_top) {
            Token* temp = pop_from_stack(&stack_top);
            free_token(temp);
        }
        return NULL;
    }


    Token* result_token = pop_from_stack(&stack_top);
    // Извлечение контейнера из токена-результата
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

// Вывод справочной информации
void print_help() {
    const char* help_text =

        "________________ СПРАВКА ПО КАЛЬКУЛЯТОРУ ________________\n"
        "\n"
        "КАК ПОЛЬЗОВАТЬСЯ:\n"
        "  Просто введите выражение и нажмите Enter.\n"
        "  Примеры: 2 + 2 * 2, sin(3.14), a = 10\n"
        "\n"
        "УПРАВЛЕНИЕ ПРОГРАММОЙ:\n"
        "  save   - сохранить всю историю введенных команд в файл 'program.txt'\n"
        "  screen - сохранить текущий вид консоли в файл 'screenshot.txt'\n"
        "  open   - загрузить и выполнить команды из файла, введенного через пробел\n"
        "  cls    - очистить экран\n"
        "  exit   - закрыть калькулятор\n"
        "  help   - показать справку по калькулятору\n"
        "\n"
        "МАТЕМАТИКА И ПЕРЕМЕННЫЕ:\n"
        "  +, -, *, /  : Стандартные операции (сложение, вычитание, умножение, деление)\n"
        "  =           : Запомнить число (пример: x = 5 + 2, теперь x равно 7)\n"
        "  ans         : Хранит результат последнего вычисления (пример: ans + 10)\n"
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
        "_________________________________________________________\n";

    print_log("%s",help_text);
}


// Вывод приветствия при запуске
void print_welcome_message()
{
    const char* welcome_msg =
        "_________________________________________________________\n"
        "        _    _                 __                        \n"
        "        |   /                /    )         /            \n"
        "        |  /     __    __   /         __   /    __       \n"
        "        | /    /___) /   ' /        /   ) /   /   '      \n"
        "________|/____(___ _(___ _(____/___(___(_/___(___ _______\n"
        "                                                         \n"
        "   Консольный Калькулятор (Числа, Векторы, Переменные)   \n"
        "_________________________________________________________\n"
        "Введите математическое выражение и нажмите Enter.\n"
        "Примеры:  2 + 2 * 2  |  x = sin(0.5)  |  v = [1, 2, 3]\n"
        "\n"
        "Введите help для полного списка команд и примеров.\n"
        "Введите exit для выхода.\n"
        "----------------------------------------------------------\n";

    print_log("%s", welcome_msg);
}


// Обновление переменной ans
void update_ans(Container* result) {
    if (!result) return;

    Container* copy = container_deep_copy(result);
    if (!copy) return;

    Token* token_val = create_token_with_container(TOK_NUMBER, NULL, copy);

    //Ищем переменную ans
    Ident* ans_ident = find_ident(FirstIdent, "ans");

    if (ans_ident) {
        // Если переменная уже есть — обновляем её значение
        if (ans_ident->value) {
            free_token(ans_ident->value); // Освобождаем старый токен
        }
        ans_ident->value = token_val;
    } else {
        // Если переменной нет — создаем новую
        Ident* new_ident = create_ident("ans", token_val);
        add_ident(&FirstIdent, new_ident);
    }
}


// Полный цикл обработки одного выражения
void process_expression(char* input) {
    //Лексический анализ
    Token *tokens = lex(input);
    if (tokens == NULL) {
        print_log("Ошибка лексического анализа\n\n");
        return;
    }

    //Сортировочная станция
    Token* rpn = shuntingYard(tokens);
    if (rpn != NULL) {
        // Вычисление
        Container* result = countRPN(rpn);

        print_log("<< ");

        update_ans(result);
        print_container(result);

        print_log("\n");

        free_container(result);
        free_tokens(rpn);
    } else {
        print_log("Ошибка синтаксического анализа\n\n");
    }

    free_tokens(tokens);
}


int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Очистка временных файлов сессии
    clear_file("session.tmp");
    clear_file("history.tmp");

    print_welcome_message();

    char input[256];

     // Основной цикл работы программы
    while (1) {
        print_log(">> ");

        if (fgets(input, sizeof(input), stdin) == NULL) break;

        // Логирование ввода пользователя
        append_to_file("session.tmp", input);

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        // Обработка системных команд
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
            print_help(); // print_help должна использовать print_log
            continue;
        }

        // Обработка "open имя_файла"
        if (strncmp(input, "open", 4) == 0) {

            char filename[256] = "program.txt"; // Дефолтное имя

            // Простая логика: если есть пробел, берем то, что после него
            char* space = strchr(input, ' ');
            if (space != NULL && strlen(space + 1) > 0) {
                strcpy(filename, space + 1);
                execute_from_file(filename);
            }
            else
            {
                print_log("Ошибка: введите название файла через пробел\n");
                print_log("Пример: open program.txt\n");
            }

            continue;
        }


        char cmd_to_save[300];
        sprintf(cmd_to_save, "%s\n", input);
        append_to_file("history.tmp", cmd_to_save);

        process_expression(input);
    }

    // Очистка ресурсов перед выходом
    remove("session.tmp");
    remove("history.tmp");
    cleanup_global_data(FirstIdent);

    return 0;
}
