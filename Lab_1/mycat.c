#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print_file(FILE *f, bool flag_n, bool flag_b, bool flag_E) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long line_no = 1;
    unsigned long nonblank_no = 1;

    while ((read = getline(&line, &len, f)) != -1) {
        bool is_blank = (read == 0) || (read == 1 && line[0] == '\n');
        if (flag_b) {
            if (!is_blank) {
                printf("%6lu\t", nonblank_no++);
            } else {
            }
        } else if (flag_n) {
            printf("%6lu\t", line_no);
        }
        if (flag_E) {
            if (read > 0 && line[read-1] == '\n') {
                fwrite(line, 1, read-1, stdout);
                fputc('$', stdout);
                fputc('\n', stdout);
            } else {
                fwrite(line, 1, read, stdout);
                fputc('$', stdout);
            }
        } else {
            fwrite(line, 1, read, stdout);
        }
        line_no++;
    }
    free(line);
}

int main(int argc, char **argv) {
    bool flag_n = false, flag_b = false, flag_E = false;
    int argi = 1;
	// парсим флаги
    while (argi < argc && argv[argi][0] == '-' && argv[argi][1] != '\0') {
        if (strcmp(argv[argi], "-") == 0) {
            break; // stdin
        }
        bool is_option = true;
        for (int i = 1; argv[argi][i]; ++i) {
            char c = argv[argi][i];
            if (c == 'n') flag_n = true;
            else if (c == 'b') flag_b = true;
            else if (c == 'E') flag_E = true;
            else {
                is_option = false;
                break;
            }
        }
        if (!is_option) break;
        argi++;
    }
	// -b приоритетнее -n
    if (flag_b) flag_n = false;

    if (argi == argc) {
        // читаем из stdin
        print_file(stdin, flag_n, flag_b, flag_E);
    } else {
        for (int i = argi; i < argc; ++i) {
            const char *fname = argv[i];
            if (strcmp(fname, "-") == 0) {
                print_file(stdin, flag_n, flag_b, flag_E);
            } else {
                FILE *f = fopen(fname, "r");
                if (!f) {
                    fprintf(stderr, "mycat: cannot open '%s'\n", fname);
                    continue;
                }
                print_file(f, flag_n, flag_b, flag_E);
                fclose(f);
            }
        }
    }
    return 0;
}

