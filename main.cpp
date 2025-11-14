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

void process_comma(Token** stack_top, Token** output_front, Token** output_rear) {
    
    
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
        return;
    }
}

void process_parenthesis(Token** stack_top, Token** output_front, Token** output_rear) {
    
    while (*stack_top && (*stack_top)->type != TOK_LPAREN) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

    
    if (!*stack_top) {
        printf("Ошибка: несогласованные круглые скобки\n");
        return;
    }

    
    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);

    
    if (*stack_top && (*stack_top)->type == TOK_FUNCTION) {
        Token* func = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, func);
    }
}

void process_vector_end(Token** stack_top, Token** output_front, Token** output_rear) {
    
    while (*stack_top && (*stack_top)->type != TOK_LBRACKET) {
        Token* op = pop_from_stack(stack_top);
        enqueue(output_front, output_rear, op);
    }

    
    if (!*stack_top) {
        printf("Ошибка: несогласованные квадратные скобки\n");
        return;
    }

    
    Token* bracket = pop_from_stack(stack_top);
    free_token(bracket);

    
    
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
                Token* c = pop_from_stack(&stack_top);
                Token* b = pop_from_stack(&stack_top);
                Token* a = pop_from_stack(&stack_top);

                Container* ac = get_container(a);
                Container* bc = get_container(b);
                Container* cc = get_container(c);

                Container* result = container_vector(ac, bc, cc);
                Token* result_token = create_token_with_container(TOK_NUMBER, NULL, result);
                push_to_stack(&stack_top, result_token);

                if(ac) free_container(ac);
                if(bc) free_container(bc);
                if(cc) free_container(cc);

                free_token(a);
                free_token(b);
                free_token(c);
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
        "Доступные команды:\n"
        "  save   - сохранить программу в program.txt\n"
        "  screen - сохранить результаты вычислений в screenshot.txt\n"
        "  open   - выполнить команды из текстового файла\n"
        "  help   - показать эту справку\n"
        "  cls    - очистить консоль\n"
        "  exit   - выход из калькулятора\n"
        "\n"
        "Доступные операторы и функции:\n"
        "  +, -, *, / - арифметические операции\n"
        "  = - присваивание (например: a = 5)\n"
        "  sin(x), cos(x), log(x), abs(x) - функции с одним аргументом\n"
        "  pow(x,y), max(x,y), cross(x,y) - функции с двумя аргументами\n"
        "  [x, y, z] - создание вектора из трех чисел\n";

    printf("%s", help_text);
    fprintf(screenshot_file, "%s", help_text);
}


int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    FILE* screenshot = fopen(screenshot_bu, "w");
    FILE* program = fopen(program_bu, "w");

    char input[256];
    Container* result;

    printf("Калькулятор запущен. Для выхода введите 'exit'. Для справки введите 'help'.\n");
    fprintf(screenshot, "Калькулятор запущен. Для выхода введите 'exit'. Для справки введите 'help'.\n");

    while (1) {
        printf(">> ");
        fprintf(screenshot, ">> ");

        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        
        fprintf(screenshot, "%s", input);

        
        input[strcspn(input, "\n")] = 0;

        
        if (strcmp(input, "exit") == 0) {
            printf("Выход из калькулятора.\n");
            fprintf(screenshot, "Выход из калькулятора.\n");
            cleanup_global_data(FirstIdent);
            break;
        }

        
        if (strcmp(input, "save") == 0) {
            copy_file(program_bu, "program.txt");
            continue;
        }

        
        if (strcmp(input, "screen") == 0) {
            fflush(screenshot);
            copy_file(screenshot_bu, "screenshot.txt");
            continue;
        }

        
        if (strcmp(input, "open") == 0) {
            execute_from_program_txt(program, screenshot);
            continue;
        }

        if (strcmp(input, "help") == 0) {
            print_help(screenshot);
            continue;
        }

        if (strcmp(input, "cls") == 0) {
            system("cls");
            fclose(screenshot);
            fclose(program);
            screenshot = fopen(screenshot_bu, "w");
            program = fopen(program_bu, "w");
            continue;
        }

        
        if (strlen(input) == 0) {
            continue;
        }

        
        
        Token *tokens = lex(input);
        if (tokens == nullptr) {
            printf("Ошибка лексического анализа\n\n");
            fprintf(screenshot, "Ошибка лексического анализа\n\n");
            continue;
        }

        
        Token* rpn = shuntingYard(tokens);
        if (rpn != nullptr) {
            
            result = countRPN(rpn);
            printf("<< ");
            fprintf(screenshot, "<< ");

            
            print_container(result);
            fputc('\n', stdout);
            
            fputc('\n', screenshot);

            
            fprintf(program, "%s\n", input);
            fflush(program);
            free_tokens(rpn);
            free_container(result);
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
