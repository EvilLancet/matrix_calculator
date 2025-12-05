#include "lib.h"

// Универсальный вывод
void print_log(const char* format, ...)
{
    va_list args;

    //Вывод на экран
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    //Вывод в файл
    FILE* f = fopen("session.tmp", "a");
    if (f) {
        va_start(args, format);
        vfprintf(f, format, args);
        va_end(args);
        fclose(f);
    }
}


//Дописать строку в конец файла
void append_to_file(const char* filename, const char* text) {
    FILE* f = fopen(filename, "a"); //
    if (f) {
        fprintf(f, "%s", text);
        fclose(f); //
    }
}


//Копирование данных из одного файла в другой
void copy_file(const char* src_name, const char* dst_name) {
    FILE* src = fopen(src_name, "rb");
    if (!src)
    {
        printf("Ошибка: не удалось открыть файл %s (возможно, он не существует).\n", src_name);
        printf("Попробуйте повторить ввод команд или перезапустите приложение\n");
        return;
    }


    FILE* dst = fopen(dst_name, "wb");
    if (!dst) {
        printf("Ошибка создания файла %s\n", dst_name);
        fclose(src);
        return;
    }

    char buffer[4096]; // Буфер на 4 КБ
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytesRead, dst);
    }

    fclose(src);
    fclose(dst);
    printf("Файл сохранен: %s\n", dst_name);
}

// Очистить файл (при запуске или cls)
void clear_file(const char* filename) {
    FILE* f = fopen(filename, "w"); // "w" перезаписывает файл пустым
    if (f) fclose(f);
}
