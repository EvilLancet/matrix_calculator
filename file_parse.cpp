#include "lib.h"



void execute_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        print_log("Ошибка: не удалось открыть файл скрипта '%s'\n", filename);
        return;
    }

    print_log("--- Начало выполнения файла %s ---\n", filename);

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Удаляем перенос строки
        line[strcspn(line, "\n")] = 0;

        // Пропускаем пустые строки
        if (strlen(line) == 0) continue;

        // Печать команды, чтобы видеть, что выполняется
        print_log(">> %s\n", line);


        // Мы не разрешаем скрипту вызывать другой скрипт, сохранять файлы или выходить.
        if (strncmp(line, "open", 4) == 0 ||
            strcmp(line, "save") == 0 ||
            strcmp(line, "screen") == 0 ||
            strcmp(line, "exit") == 0 ||
            strcmp(line, "cls") == 0) {

            print_log("<< Команда пропущена (безопасность)\n\n");
            continue;
        }

        // выполение команды
        process_expression(line);

        char cmd_with_newline[300];
        sprintf(cmd_with_newline, "%s\n", line);
        append_to_file("history.tmp", cmd_with_newline);
    }

    fclose(file);
    print_log("--- Конец выполнения файла %s ---\n", filename);
}
