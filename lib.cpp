#include "lib.h"

// Умный вывод числа: целые выводятся без дробной части
void print_smart_double(double value) {
    double int_part;
     // Проверка, является ли число целым (дробная часть близка к 0)
    if (fabs(modf(value, &int_part)) < 1e-9) {

        print_log("%.0f", value);
    } else {

        print_log("%g", value);
    }
}

// Полная очистка списка переменных
void cleanup_global_data(Ident* FirstIdent) {

    Ident* current = FirstIdent;
    while (current != NULL) {
        Ident* next = current->next;

        // Освобождение имени
        if (current->name) {
            free(current->name);
        }


        if (current->value) {
            free_token(current->value);
        }


        free(current);
        current = next;
    }
    FirstIdent = NULL;
}



// Инициализация переменной
Ident* create_ident(char *name, Token *value)
{
    Ident *new_ident = (Ident*)malloc(sizeof(Ident));
    if (!new_ident) return nullptr;

    new_ident->name = strdup(name);
    new_ident->value = value;
    new_ident->prev = nullptr;
    new_ident->next = nullptr;

    return new_ident;
}

// Добавление переменной в начало связного списка
void add_ident(Ident **first, Ident *new_ident) {
    if (new_ident == nullptr) return;

    new_ident->next = *first;
    new_ident->prev = nullptr;

    if (*first)
        (*first)->prev = new_ident;

    *first = new_ident;
}

// Удаление переменной из двусвязного списка
void remove_ident(Ident **first, Ident *ident_to_remove) {
    if (!first || !*first || !ident_to_remove) return;


    if (*first == ident_to_remove) {
        *first = ident_to_remove->next;
    }


    if (ident_to_remove->prev)
        ident_to_remove->prev->next = ident_to_remove->next;

    if (ident_to_remove->next)
        ident_to_remove->next->prev = ident_to_remove->prev;


    free(ident_to_remove->name);
    free(ident_to_remove);
}

// Линейный поиск переменной по имени
Ident* find_ident(Ident *first, const char *name) {
    Ident *current = first;

    while (current) {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }

    return nullptr;
}




// Обертки для освобождения памяти конкретных типов данных
void free_int_container(void *data) {
    if (data) free(data);
}

void free_float_container(void *data) {
    if (data) free(data);
}

void free_string_container(void *data) {
    StringContainer *sc = (StringContainer*)data;
    if (sc) {
        free(sc->value);
        free(sc);
    }
}

void free_vector_container(void *data) {
    if (data) free(data);
}



// Реализация вывода для различных типов данных
void print_int_container(void *data) {
    if (data) {
        IntContainer *ic = (IntContainer*)data;
        print_log("%d", ic->value);
    }
}

void print_float_container(void *data) {
    if (data) {
        FloatContainer *fc = (FloatContainer*)data;
        print_smart_double(fc->value);
    }
}

void print_string_container(void *data) {
    if (data) {
        StringContainer *sc = (StringContainer*)data;
        print_log("%s", sc->value);
    }
}

void print_vector_container(void *data) {
    if (data) {
        VectorContainer *vc = (VectorContainer*)data;
        print_log("[");
        print_smart_double(vc->x);
        print_log(", ");
        print_smart_double(vc->y);
        print_log(", ");
        print_smart_double(vc->z);
        print_log("]");
    }
}


// Инициализация int контейнера
Container* create_int_container(int value) {
    Container *container = (Container*)malloc(sizeof(Container));
    IntContainer *data = (IntContainer*)malloc(sizeof(IntContainer));

    data->value = value;
    container->type = CT_INT;
    container->data = data;
    container->free_func = free_int_container;
    container->print_func = print_int_container;

    return container;
}

// Инициализация float контейнера
Container* create_float_container(double value) {
    Container *container = (Container*)malloc(sizeof(Container));
    FloatContainer *data = (FloatContainer*)malloc(sizeof(FloatContainer));

    data->value = value;
    container->type = CT_FLOAT;
    container->data = data;
    container->free_func = free_float_container;
    container->print_func = print_float_container;

    return container;
}

// Инициализация строкового контейнера
Container* create_string_container(const char *value) {
    Container *container = (Container*)malloc(sizeof(Container));
    StringContainer *data = (StringContainer*)malloc(sizeof(StringContainer));

    data->value = strdup(value);
    data->length = strlen(value);
    container->type = CT_STRING;
    container->data = data;
    container->free_func = free_string_container;
    container->print_func = print_string_container;

    return container;
}

// Инициализация векторного контейнера
Container* create_vector_container(double x, double y, double z) {
    Container *container = (Container*)malloc(sizeof(Container));
    VectorContainer *data = (VectorContainer*)malloc(sizeof(VectorContainer));

    data->x = x;
    data->y = y;
    data->z = z;
    container->type = CT_VECTOR;
    container->data = data;
    container->free_func = free_vector_container;
    container->print_func = print_vector_container;

    return container;
}


// Уничтожение контейнера с использованием его внутренней функции очистки
void free_container(Container *container) {
    if (container) {
        if (container->free_func && container->data) {
            container->free_func(container->data);
        }
        free(container);
    }
}

// Вывод содержимого контейнера с использованием его внутренней функции очистки
void print_container(Container *container) {
    if (container && container->print_func) {

        container->print_func(container->data);
    }
}

// Клонирование контейнера
Container* container_deep_copy(Container *src) {
    if (!src) return NULL;

    switch (src->type) {
        case CT_INT: {
            IntContainer *ic = (IntContainer*)src->data;
            return create_int_container(ic->value);
        }
        case CT_FLOAT: {
            FloatContainer *fc = (FloatContainer*)src->data;
            return create_float_container(fc->value);
        }
        case CT_STRING: {
            StringContainer *sc = (StringContainer*)src->data;
            return create_string_container(sc->value);
        }
        case CT_VECTOR: {
            VectorContainer *vc = (VectorContainer*)src->data;
            return create_vector_container(vc->x, vc->y, vc->z);
        }
        default:
            return NULL;
    }
}


// Сравнение значений двух контейнеров
int container_compare(Container *a, Container *b) {
    if (!a || !b) return 0;
    if (a->type != b->type) return 0;

    switch (a->type) {
        case CT_INT: {
            IntContainer *ia = (IntContainer*)a->data;
            IntContainer *ib = (IntContainer*)b->data;
            return ia->value == ib->value;
        }
        case CT_FLOAT: {
            FloatContainer *fa = (FloatContainer*)a->data;
            FloatContainer *fb = (FloatContainer*)b->data;
            return fabs(fa->value - fb->value) < 1e-10;
        }
        case CT_STRING: {
            StringContainer *sa = (StringContainer*)a->data;
            StringContainer *sb = (StringContainer*)b->data;
            return strcmp(sa->value, sb->value) == 0;
        }
        case CT_VECTOR: {
            VectorContainer *va = (VectorContainer*)a->data;
            VectorContainer *vb = (VectorContainer*)b->data;
            return fabs(va->x - vb->x) < 1e-10 &&
                   fabs(va->y - vb->y) < 1e-10 &&
                   fabs(va->z - vb->z) < 1e-10;
        }
        default:
            return 0;
    }
}




//Cоздание токена
Token *create_token(TokenT type, const char *value) {
    Token *token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = value ? strdup(value) : NULL;
    token->container = NULL;
    token->prev = NULL;
    token->next = NULL;
    return token;
}


// Создание токена c привязкой контейнера
Token *create_token_with_container(TokenT type, const char *value, Container *container) {
    Token *token = create_token(type, value);
    if (token) {
        token->container = container;
    }
    return token;
}

// Добавление токена в конец списка
void add_token(Token **head, Token **tail, Token *token) {
    if (token == NULL) return;

    if (*head == NULL) {
        *head = token;
        *tail = token;
    } else {
        (*tail)->next = token;
        token->prev = *tail;
        *tail = token;
    }
}

// Очистка токена и его содержимого
void free_token(Token *token) {
    if (token == NULL) return;


    if (token->value) {
        free(token->value);
    }


    if (token->container) {
        free_container(token->container);
    }

    free(token);
}

// Очистка всего списка токенов
void free_tokens(Token *head) {
    Token *current = head;
    while (current != NULL) {
        Token *next = current->next;
        free_token(current);
        current = next;
    }
}

// Присвоение контейнера токену
void token_set_container(Token *token, Container *container) {
    if (token == NULL) return;


    if (token->container) {
        free_container(token->container);
    }

    token->container = container;
}


// Получение числа из строки и создание токена с соответствующим контейнером
Token *create_number_token(const char *value) {
    if (value == NULL) return NULL;

    Token *token = create_token(TOK_NUMBER, value);
    if (token) {

        if (strchr(value, '.') != NULL) {
            double float_value = atof(value);

            token->container = create_float_container(float_value);
        } else {
            int int_value = atoi(value);

            token->container = create_int_container(int_value);
        }
    }
    return token;
}

//Копия токена с глубоким копированием данных
Token *copy_token(const Token *src) {
    if (src == NULL) return NULL;

    Token *copy = create_token(src->type, src->value);
    if (copy && src->container) {
        copy->container = container_deep_copy(src->container);
    }
    return copy;
}

// Добавление токена на вершину стека
void push_to_stack(Token** stack_top, Token* item)
{
    if (stack_top == nullptr || item == nullptr) return;

    item->prev = nullptr;
    item->next = *stack_top;

    if (*stack_top != nullptr) {
        (*stack_top)->prev = item;
    }

    *stack_top = item;
}

// Извлечение элемента с вершины стека
Token* pop_from_stack(Token** stack_top)
{
    if (stack_top == nullptr || *stack_top == nullptr) {
        return nullptr;
    }

    Token* popped = *stack_top;
    *stack_top = (*stack_top)->next;

    if (*stack_top != nullptr) {
        (*stack_top)->prev = nullptr;
    }

    popped->next = nullptr;
    popped->prev = nullptr;
    return popped;
}


// Добавление элемента в очередь
void enqueue(Token** queue_front, Token** queue_rear, Token* item)
{
    if (item == nullptr) return;

    item->next = nullptr;
    item->prev = *queue_rear;

    if (*queue_rear != nullptr) {
        (*queue_rear)->next = item;
    }

    *queue_rear = item;

    if (*queue_front == nullptr) {
        *queue_front = item;
    }
}

// Извлечение элемента из очереди
Token* dequeue(Token** queue_front, Token** queue_rear)
{
    if (queue_front == nullptr || *queue_front == nullptr) {
        return nullptr;
    }

    Token* dequeued = *queue_front;
    *queue_front = (*queue_front)->next;

    if (*queue_front != nullptr) {
        (*queue_front)->prev = nullptr;
    } else {
        *queue_rear = nullptr;
    }

    dequeued->next = nullptr;
    dequeued->prev = nullptr;
    return dequeued;
}



// Приведение числового контейнера к double
double container_to_double(Container* container) {
    if (!container) return 0.0;

    switch (container->type) {
        case CT_INT: {
            IntContainer* ic = (IntContainer*)container->data;
            return (double)ic->value;
        }
        case CT_FLOAT: {
            FloatContainer* fc = (FloatContainer*)container->data;
            return fc->value;
        }
        default:
            return 0.0;
    }
}

// Вычисление синуса
Container* sin_func(Container** args, int arg_count) {

    if (!args[0]) return NULL;
    if (args[0]->type == CT_INT || args[0]->type == CT_FLOAT)
    {
        double value = container_to_double(args[0]);
        return create_float_container(sin(value));
    }
    return NULL;
}

// Вычисление косинуса
Container* cos_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        print_log("cos: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;
    if (args[0]->type != CT_INT && args[0]->type != CT_FLOAT) {
        print_log("cos: аргумент должен быть числом\n");
        return NULL;
    }

    double value = container_to_double(args[0]);
    return create_float_container(cos(value));
}


// Натуральный логарифм с проверкой области определения
Container* log_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        print_log("log: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;
    if (args[0]->type != CT_INT && args[0]->type != CT_FLOAT) {
        print_log("log: аргумент должен быть числом\n");
        return NULL;
    }

    double value = container_to_double(args[0]);
    if (value <= 0) {
        print_log("log: аргумент должен быть положительным\n");
        return NULL;
    }
    return create_float_container(log(value));
}


// Возведение в степень
Container* pow_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        print_log("pow: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if ((args[0]->type != CT_INT && args[0]->type != CT_FLOAT) ||
        (args[1]->type != CT_INT && args[1]->type != CT_FLOAT)) {
        print_log("pow: аргументы должны быть числами\n");
        return NULL;
    }

    double base = container_to_double(args[0]);
    double exponent = container_to_double(args[1]);

    if (base == 0 && exponent < 0) {
        print_log("pow: деление на ноль\n");
        return NULL;
    }
    return create_float_container(pow(base, exponent));
}


// Выбор максимального из двух чисел
Container* max_func(Container* args[], int arg_count) {
    if (arg_count != 2) {
        print_log("max: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if ((args[0]->type != CT_INT && args[0]->type != CT_FLOAT) ||
        (args[1]->type != CT_INT && args[1]->type != CT_FLOAT)) {
        print_log("max: аргументы должны быть числами\n");
        return NULL;
    }

    double a = container_to_double(args[0]);
    double b = container_to_double(args[1]);
    return create_float_container(a > b ? a : b);
}


// Векторное произведение
Container* cross_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        print_log("cross: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if (args[0]->type != CT_VECTOR || args[1]->type != CT_VECTOR) {
        print_log("cross: оба аргумента должны быть векторами\n");
        return NULL;
    }

    VectorContainer* v1 = (VectorContainer*)args[0]->data;
    VectorContainer* v2 = (VectorContainer*)args[1]->data;


    double x = v1->y * v2->z - v1->z * v2->y;
    double y = v1->z * v2->x - v1->x * v2->z;
    double z = v1->x * v2->y - v1->y * v2->x;

    return create_vector_container(x,y,z);
}

Container* abs_func(Container** args, int arg_count)
{
    if (arg_count != 1) {
        print_log("length: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;

    if (args[0]->type == CT_INT || args[0]->type == CT_FLOAT)
    {
        double value = container_to_double(args[0]);
        return create_float_container(fabs(value));
    }

    if (args[0]->type != CT_VECTOR) {
        print_log("length: аргумент должен быть вектором\n");
        return NULL;
    }

    VectorContainer* vec = (VectorContainer*)args[0]->data;


    double length = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    return create_float_container(length);
}


// Операция вычитания
Container* sub_func(Container** args, int arg_count)
{
    if (arg_count != 2) {
        print_log("Вычитание: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        print_log("Вычитание: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];


    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) - container_to_double(b);
        return create_float_container(result);
    }


    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(va->x - vb->x, va->y - vb->y, va->z - vb->z);
    }

    print_log("Ошибка: несовместимые типы для вычитания\n");
    return NULL;
}


// Операция сложения
Container* add_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        print_log("Сложение: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        print_log("Сложение: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];


    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) + container_to_double(b);
        return create_float_container(result);
    }


    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(va->x + vb->x, va->y + vb->y, va->z + vb->z);
    }

    print_log("Ошибка: несовместимые типы для сложения\n");
    return NULL;
}

// Унарный минус
Container* neg_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        print_log("Унарный минус: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) {
        print_log("Унарный минус: аргумент не может быть NULL\n");
        return NULL;
    }

    Container* a = args[0];

    switch (a->type) {
        case CT_INT: {
            IntContainer* ic = (IntContainer*)a->data;
            return create_int_container(-ic->value);
        }
        case CT_FLOAT: {
            FloatContainer* fc = (FloatContainer*)a->data;
            return create_float_container(-fc->value);
        }
        case CT_VECTOR: {
            VectorContainer* vc = (VectorContainer*)a->data;
            return create_vector_container(-vc->x, -vc->y, -vc->z);
        }
        default:
            print_log("Ошибка: унарный минус не применим к данному типу\n");
            return NULL;
    }
}


// Деление
Container* div_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        print_log("Деление: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        print_log("Деление: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];


    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double divisor = container_to_double(b);
        if (divisor == 0.0) {
            print_log("Ошибка: деление на ноль\n");
            return NULL;
        }
        double result = container_to_double(a) / divisor;
        return create_float_container(result);
    }


    if (a->type == CT_VECTOR && (b->type == CT_INT || b->type == CT_FLOAT)) {
        double divisor = container_to_double(b);
        if (divisor == 0.0) {
            print_log("Ошибка: деление на ноль\n");
            return NULL;
        }
        VectorContainer* va = (VectorContainer*)a->data;
        return create_vector_container(va->x / divisor, va->y / divisor, va->z / divisor);
    }

    print_log("Ошибка: несовместимые типы для деления\n");
    return NULL;
}


// Универсальное умножение
Container* mul_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        print_log("Умножение: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        print_log("Умножение: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];


    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) * container_to_double(b);
        return create_float_container(result);
    }


    if (a->type == CT_VECTOR && (b->type == CT_INT || b->type == CT_FLOAT)) {
        VectorContainer* va = (VectorContainer*)a->data;
        double scalar = container_to_double(b);
        return create_vector_container(va->x * scalar, va->y * scalar, va->z * scalar);
    }


    if ((a->type == CT_INT || a->type == CT_FLOAT) && b->type == CT_VECTOR) {
        double scalar = container_to_double(a);
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(scalar * vb->x, scalar * vb->y, scalar * vb->z);
    }


    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_float_container(va->x * vb->x + va->y * vb->y + va->z * vb->z);
    }

    print_log("Ошибка: несовместимые типы для умножения\n");
    return NULL;
}

