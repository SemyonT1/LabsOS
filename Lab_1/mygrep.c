#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define COLOR_RED   "\033[1;31m"
#define COLOR_RESET "\033[0m"

void print_highlighted(const char *line, const char *pattern) {
    const char *p = line;
    size_t pat_len = strlen(pattern);

    while (*p) {
        const char *match = strstr(p, pattern);
        if (match) {
            // печатаем всё до совпадения
            fwrite(p, 1, match - p, stdout);
            // печатаем совпадение с подсветкой
            printf(COLOR_RED "%.*s" COLOR_RESET, (int)pat_len, match);
            // двигаем указатель дальше
            p = match + pat_len;
        } else {
            // оставшаяся часть строки
            fputs(p, stdout);
            break;
        }
    }
}

void grep_stream(FILE *f, const char *pattern, bool flag_n, const char *filename_if_any) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long line_no = 1;

    while ((read = getline(&line, &len, f)) != -1) {
        if (strstr(line, pattern) != NULL) {
            if (filename_if_any) {
                printf("%s:", filename_if_any);
            }
            if (flag_n) {
                printf("%lu:", line_no);
            }
            // печатаем с подсветкой
            print_highlighted(line, pattern);
            // если в конце не было \n — добавляем
            if (read > 0 && line[read-1] != '\n') putchar('\n');
        }
        line_no++;
    }
    free(line);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Верное использование команды: %s [-n] паттерн [имя файла...]\n", argv[0]);
        return 2;
    }

    bool flag_n = false;
    int argi = 1;

    // парсинг флагов
    while (argi < argc && argv[argi][0] == '-' && argv[argi][1] != '\0') {
        if (strcmp(argv[argi], "-") == 0) break;
        bool is_option = true;
        for (int i = 1; argv[argi][i]; ++i) {
            char c = argv[argi][i];
            if (c == 'n') flag_n = true;
            else { is_option = false; break; }
        }
        if (!is_option) break;
        argi++;
    }

    if (argi >= argc) {
        fprintf(stderr, "mygrep: паттерн потерян\n");
        return 2;
    }

    const char *pattern = argv[argi++];
    if (argi == argc) {
        grep_stream(stdin, pattern, flag_n, NULL);
    } else {
        for (int i = argi; i < argc; ++i) {
            const char *fname = argv[i];
            if (strcmp(fname, "-") == 0) {
                grep_stream(stdin, pattern, flag_n, NULL);
            } else {
                FILE *f = fopen(fname, "r");
                if (!f) {
                    fprintf(stderr, "mygrep: невозможно открыть '%s'\n", fname);
                    continue;
                }
                grep_stream(f, pattern, flag_n, fname);
                fclose(f);
            }
        }
    }
    return 0;
}

