#include "lib.h"

// Универсальный принтер: пишет и в консоль, и в session.tmp
void print_log(const char* format, ...)
{
    va_list args;

    // 1. Вывод на экран
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // 2. Вывод в файл
    FILE* f = fopen("session.tmp", "a");
    if (f) {
        va_start(args, format);
        vfprintf(f, format, args);
        va_end(args);
        fclose(f);
    }
}


// Функция 1: Дописать строку в конец файла
void append_to_file(const char* filename, const char* text) {
    FILE* f = fopen(filename, "a"); // "a" = append (дописать)
    if (f) {
        fprintf(f, "%s", text);
        fclose(f); // Сразу закрываем!
    }
}

// Функция 2: Скопировать один файл в другой (для команд save/screen)
void copy_file(const char* src_name, const char* dst_name) {
    FILE* src = fopen(src_name, "r");
    if (!src) {
        printf("Нечего сохранять (файл %s пуст).\n", src_name);
        return;
    }
    FILE* dst = fopen(dst_name, "w");
    if (!dst) {
        printf("Ошибка создания файла %s\n", dst_name);
        fclose(src);
        return;
    }

    char ch;
    while ((ch = fgetc(src)) != EOF) {
        fputc(ch, dst);
    }

    fclose(src);
    fclose(dst);
    printf("Файл сохранен: %s\n", dst_name);
}

// Функция 3: Очистить файл (при запуске или cls)
void clear_file(const char* filename) {
    FILE* f = fopen(filename, "w"); // "w" перезаписывает файл пустым
    if (f) fclose(f);
}
