#include "lib.h"

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

    char filename[256];

    printf("Введите название файла: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    FILE* input_file = fopen(filename, "r");
    if (input_file == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", filename);
        return;
    }

    char line[256];
    Container* result;

    printf("Выполнение команд из %s...\n", filename);
    fprintf(screenshot_file, "Выполнение команд из %s...\n", filename);

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
    printf("Выполнение команд из завершено.\n");
    fprintf(screenshot_file, "Выполнение команд завершено.\n");
}
