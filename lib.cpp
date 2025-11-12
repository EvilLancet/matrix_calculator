#include "lib.h"

// ------------ функции для работы функций --------------------


// ------------ функции для работы с переменными --------------

Ident* create_ident(char *name, Token *value)
{
    Ident *new_ident = (Ident*)malloc(sizeof(Ident));
    if (!new_ident) return nullptr;

    new_ident->name = strdup(name); // Копируем строку
    new_ident->value = value;
    new_ident->prev = nullptr;
    new_ident->next = nullptr;

    return new_ident;
}

// Добавление в начало списка
void add_ident(Ident **first, Ident *new_ident) {
    if (new_ident == nullptr) return;

    new_ident->next = *first;
    new_ident->prev = nullptr;

    if (*first)
        (*first)->prev = new_ident;

    *first = new_ident;
}

// Удаление идентификатора из списка
void remove_ident(Ident **first, Ident *ident_to_remove) {
    if (!first || !*first || !ident_to_remove) return;

    // Если удаляемый элемент - первый
    if (*first == ident_to_remove) {
        *first = ident_to_remove->next;
    }

    // Перестраиваем связи
    if (ident_to_remove->prev)
        ident_to_remove->prev->next = ident_to_remove->next;

    if (ident_to_remove->next)
        ident_to_remove->next->prev = ident_to_remove->prev;

    // Очищаем память
    free(ident_to_remove->name);
    free(ident_to_remove);
}

// Поиск идентификатора по имени
Ident* find_ident(Ident *first, const char *name) {
    Ident *current = first;

    while (current) {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }

    return nullptr; // Не найден
}

// ------------ функции для работы с контейнерами --------------


// Функции освобождения данных контейнеров
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

// Функции печати контейнеров
void print_int_container(void *data) {
    if (data) {
        IntContainer *ic = (IntContainer*)data;
        printf("%d", ic->value);
    }
}

void print_float_container(void *data) {
    if (data) {
        FloatContainer *fc = (FloatContainer*)data;
        printf("%.6f", fc->value);
    }
}

void print_string_container(void *data) {
    if (data) {
        StringContainer *sc = (StringContainer*)data;
        printf("%s", sc->value);
    }
}

void print_vector_container(void *data) {
    if (data) {
        VectorContainer *vc = (VectorContainer*)data;
        printf("[%.6f, %.6f, %.6f]", vc->x, vc->y, vc->z);
    }
}

// Функции создания контейнеров
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

// Функция для освобождения контейнера
void free_container(Container *container) {
    if (container) {
        if (container->free_func && container->data) {
            container->free_func(container->data);
        }
        free(container);
    }
}

// Функция для печати контейнера
void print_container(Container *container) {
    if (container && container->print_func) {
        container->print_func(container->data);
    }
}

// Дополнительные утилиты для работы с контейнерами
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


// ------------ функции для работы с токенами --------------


Token *create_token(TokenT type, const char *value) {
    Token *token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = value ? strdup(value) : NULL;
    token->container = NULL;  // Инициализируем контейнер как NULL
    token->prev = NULL;
    token->next = NULL;
    return token;
}

/*
// Функция для создания нового токена
Token *create_token(TokenT type, const char *value) {
    Token *token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    token->prev = nullptr;
    token->next = nullptr;
    return token;
}
*/


// Функция для создания токена с контейнером
Token *create_token_with_container(TokenT type, const char *value, Container *container) {
    Token *token = create_token(type, value);
    if (token) {
        token->container = container;
    }
    return token;
}

// Функция для добавления токена в конец списка
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

// Функция для освобождения памяти одного токена
void free_token(Token *token) {
    if (token == NULL) return;

    // Освобождаем значение строки
    if (token->value) {
        free(token->value);
    }

    // Освобождаем контейнер, если он есть
    if (token->container) {
        free_container(token->container);
    }

    free(token);
}

// Функция для освобождения памяти всего списка токенов
void free_tokens(Token *head) {
    Token *current = head;
    while (current != NULL) {
        Token *next = current->next;
        free_token(current);
        current = next;
    }
}

// Функция для установки контейнера в токен
void token_set_container(Token *token, Container *container) {
    if (token == NULL) return;

    // Освобождаем старый контейнер, если он был
    if (token->container) {
        free_container(token->container);
    }

    token->container = container;
}

// Функция для получения контейнера из токена
Container *token_get_container(const Token *token) {
    return token ? token->container : NULL;
}

// Функция для создания токена числа (автоматически определяет int/float)
Token *create_number_token(const char *value) {
    if (value == NULL) return NULL;

    Token *token = create_token(TOK_NUMBER, value);
    if (token) {
        // Проверяем, содержит ли строка точку (дробное число)
        if (strchr(value, '.') != NULL) {
            double float_value = atof(value);
            //printf("%f \n", float_value);
            token->container = create_float_container(float_value);
        } else {
            int int_value = atoi(value);
            //printf("%d \n", int_value);
            token->container = create_int_container(int_value);
        }
    }
    return token;
}

// Функция для создания токена строки
Token *create_string_token(TokenT type, const char *value) {
    if (value == NULL) return NULL;

    Token *token = create_token(type, value);
    if (token) {
        token->container = create_string_container(value);
    }
    return token;
}

// Функция для создания токена вектора
Token *create_vector_token(double x, double y, double z) {
    Token *token = create_token(TOK_VECTOR, NULL); // или специальный тип для вектора
    if (token) {
        token->container = create_vector_container(x, y, z);
    }
    return token;
}

// Функция для копирования токена (глубокая копия)
Token *copy_token(const Token *src) {
    if (src == NULL) return NULL;

    Token *copy = create_token(src->type, src->value);
    if (copy && src->container) {
        copy->container = container_deep_copy(src->container);
    }
    return copy;
}

// Функция для печати токена (для отладки)
void print_token(const Token *token) {
    if (token == NULL) {
        printf("NULL_TOKEN");
        return;
    }

    printf("Token{type=%d, container=", token->type);

    if (token->container) {
        print_container(token->container);
    } else {
        printf("NULL");
    }

    printf("}\n");
}

// ------------ функции для работы со стеком --------------

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

// ------------ функции для работы с очередью --------------


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

// ------------ вспомогательные функции --------------


// Функция преобразования контейнера в double
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


Container* sin_func(Container** args, int arg_count) {

    if (!args[0]) return NULL;
    if (args[0]->type == CT_INT || args[0]->type == CT_FLOAT)
    {
        double value = container_to_double(args[0]);
        return create_float_container(sin(value));
    }
    return NULL;
}

Container* cos_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        printf("cos: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;
    if (args[0]->type != CT_INT && args[0]->type != CT_FLOAT) {
        printf("cos: аргумент должен быть числом\n");
        return NULL;
    }

    double value = container_to_double(args[0]);
    return create_float_container(cos(value));
}

Container* log_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        printf("log: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;
    if (args[0]->type != CT_INT && args[0]->type != CT_FLOAT) {
        printf("log: аргумент должен быть числом\n");
        return NULL;
    }

    double value = container_to_double(args[0]);
    if (value <= 0) {
        printf("log: аргумент должен быть положительным\n");
        return NULL;
    }
    return create_float_container(log(value));
}

Container* pow_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        printf("pow: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if ((args[0]->type != CT_INT && args[0]->type != CT_FLOAT) ||
        (args[1]->type != CT_INT && args[1]->type != CT_FLOAT)) {
        printf("pow: аргументы должны быть числами\n");
        return NULL;
    }

    double base = container_to_double(args[0]);
    double exponent = container_to_double(args[1]);

    if (base == 0 && exponent < 0) {
        printf("pow: деление на ноль\n");
        return NULL;
    }
    return create_float_container(pow(base, exponent));
}

Container* max_func(Container* args[], int arg_count) {
    if (arg_count != 2) {
        printf("max: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if ((args[0]->type != CT_INT && args[0]->type != CT_FLOAT) ||
        (args[1]->type != CT_INT && args[1]->type != CT_FLOAT)) {
        printf("max: аргументы должны быть числами\n");
        return NULL;
    }

    double a = container_to_double(args[0]);
    double b = container_to_double(args[1]);
    return create_float_container(a > b ? a : b);
}

Container* cross_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        printf("cross: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) return NULL;
    if (args[0]->type != CT_VECTOR || args[1]->type != CT_VECTOR) {
        printf("cross: оба аргумента должны быть векторами\n");
        return NULL;
    }

    VectorContainer* v1 = (VectorContainer*)args[0]->data;
    VectorContainer* v2 = (VectorContainer*)args[1]->data;

    // Вычисление векторного произведения
    double x = v1->y * v2->z - v1->z * v2->y;
    double y = v1->z * v2->x - v1->x * v2->z;
    double z = v1->x * v2->y - v1->y * v2->x;

    return create_vector_container(x,y,z);
}

Container* abs_func(Container** args, int arg_count)
{
    if (arg_count != 1) {
        printf("length: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) return NULL;
    if (args[0]->type != CT_VECTOR) {
        printf("length: аргумент должен быть вектором\n");
        return NULL;
    }

    VectorContainer* vec = (VectorContainer*)args[0]->data;

    // Вычисление длины вектора
    double length = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    return create_float_container(length);
}

Container* sub_func(Container** args, int arg_count)
{
    if (arg_count != 2) {
        printf("Вычитание: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        printf("Вычитание: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];

    // Числа
    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) - container_to_double(b);
        return create_float_container(result);
    }

    // Векторы
    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(va->x - vb->x, va->y - vb->y, va->z - vb->z);
    }

    printf("Ошибка: несовместимые типы для вычитания\n");
    return NULL;
}

Container* add_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        printf("Сложение: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        printf("Сложение: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];

    // Числа
    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) + container_to_double(b);
        return create_float_container(result);
    }

    // Векторы
    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(va->x + vb->x, va->y + vb->y, va->z + vb->z);
    }

    printf("Ошибка: несовместимые типы для сложения\n");
    return NULL;
}


Container* neg_func(Container** args, int arg_count) {
    if (arg_count != 1) {
        printf("Унарный минус: ожидается 1 аргумент\n");
        return NULL;
    }
    if (!args[0]) {
        printf("Унарный минус: аргумент не может быть NULL\n");
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
            printf("Ошибка: унарный минус не применим к данному типу\n");
            return NULL;
    }
}

Container* div_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        printf("Деление: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        printf("Деление: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];

    // Числа
    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double divisor = container_to_double(b);
        if (divisor == 0.0) {
            printf("Ошибка: деление на ноль\n");
            return NULL;
        }
        double result = container_to_double(a) / divisor;
        return create_float_container(result);
    }

    // Вектор / число
    if (a->type == CT_VECTOR && (b->type == CT_INT || b->type == CT_FLOAT)) {
        double divisor = container_to_double(b);
        if (divisor == 0.0) {
            printf("Ошибка: деление на ноль\n");
            return NULL;
        }
        VectorContainer* va = (VectorContainer*)a->data;
        return create_vector_container(va->x / divisor, va->y / divisor, va->z / divisor);
    }

    printf("Ошибка: несовместимые типы для деления\n");
    return NULL;
}

Container* mul_func(Container** args, int arg_count) {
    if (arg_count != 2) {
        printf("Умножение: ожидается 2 аргумента\n");
        return NULL;
    }
    if (!args[0] || !args[1]) {
        printf("Умножение: аргументы не могут быть NULL\n");
        return NULL;
    }

    Container* a = args[0];
    Container* b = args[1];

    // Числа
    if ((a->type == CT_INT || a->type == CT_FLOAT) &&
        (b->type == CT_INT || b->type == CT_FLOAT)) {
        double result = container_to_double(a) * container_to_double(b);
        return create_float_container(result);
    }

    // Вектор * число
    if (a->type == CT_VECTOR && (b->type == CT_INT || b->type == CT_FLOAT)) {
        VectorContainer* va = (VectorContainer*)a->data;
        double scalar = container_to_double(b);
        return create_vector_container(va->x * scalar, va->y * scalar, va->z * scalar);
    }

    // Число * вектор
    if ((a->type == CT_INT || a->type == CT_FLOAT) && b->type == CT_VECTOR) {
        double scalar = container_to_double(a);
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_vector_container(scalar * vb->x, scalar * vb->y, scalar * vb->z);
    }

    // Скалярное произведение векторов
    if (a->type == CT_VECTOR && b->type == CT_VECTOR) {
        VectorContainer* va = (VectorContainer*)a->data;
        VectorContainer* vb = (VectorContainer*)b->data;
        return create_float_container(va->x * vb->x + va->y * vb->y + va->z * vb->z);
    }

    printf("Ошибка: несовместимые типы для умножения\n");
    return NULL;
}
