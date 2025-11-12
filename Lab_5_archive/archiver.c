#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 4096

// структура заголовка файла 
struct file_entry {
    char name[256];
    mode_t mode;
    off_t size;
    time_t mtime;
};
//  Добавление файла в архив
	void archive_add(const char *archive_name, const char *src_name) {
    // Пробуем удалить старую версию, если она есть
    int arch_fd = open(archive_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (arch_fd < 0) {
        perror("open archive");
        exit(EXIT_FAILURE);
    }

    int src_fd = open(src_name, O_RDONLY);
    if (src_fd < 0) {
        perror("open source");
        close(arch_fd);
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(src_fd, &st) < 0) {
        perror("fstat");
        close(src_fd);
        close(arch_fd);
        exit(EXIT_FAILURE);
    }

    struct file_entry entry = {0};
    strncpy(entry.name, src_name, sizeof(entry.name) - 1);
    entry.name[sizeof(entry.name) - 1] = '\0';
    entry.mode = st.st_mode;
    entry.size = st.st_size;
    entry.mtime = st.st_mtime;

    if (write(arch_fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("write header");
        close(src_fd);
        close(arch_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = write(arch_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("write data");
            close(src_fd);
            close(arch_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0)
        perror("read source");

    close(src_fd);
    close(arch_fd);
    printf("Added: %s\n", src_name);
}

//  Вывод содержимого архива
void archive_show(const char *archive_name) {
    int arch_fd = open(archive_name, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive");
        exit(EXIT_FAILURE);
    }

    struct file_entry entry;
    int empty = 1;

    printf("Archive contents (%s):\n", archive_name);

    while (read(arch_fd, &entry, sizeof(entry)) == sizeof(entry)) {
        empty = 0;
        printf("%-20s %10ld bytes  %s", entry.name, (long)entry.size, ctime(&entry.mtime));
        if (lseek(arch_fd, entry.size, SEEK_CUR) == (off_t)-1) {
            perror("lseek");
            break;
        }
    }

    if (empty)
        printf("  [empty archive]\n");

    close(arch_fd);
}

//  Удаление файла из архива
void archive_extract(const char *archive_name, const char *target_name) {
    int arch_fd = open(archive_name, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive");
        exit(EXIT_FAILURE);
    }

    int temp_fd = open("tmp_archive.arc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("create temp");
        close(arch_fd);
        exit(EXIT_FAILURE);
    }

    struct file_entry entry;
    char buffer[BUFFER_SIZE];
    int found = 0;

    while (1) {
        ssize_t hdr_bytes = read(arch_fd, &entry, sizeof(entry));
        if (hdr_bytes == 0) break;                     // конец файла
        if (hdr_bytes != sizeof(entry)) break;          // ошибка

        if (strcmp(entry.name, target_name) == 0) {
            // Извлекаем
            found = 1;
            int out_fd = open(entry.name, O_WRONLY | O_CREAT | O_TRUNC, entry.mode);
            if (out_fd < 0) {
                perror("extract");
                close(arch_fd);
                close(temp_fd);
                exit(EXIT_FAILURE);
            }

            off_t remaining = entry.size;
            while (remaining > 0) {
                ssize_t chunk = (remaining > (off_t)sizeof(buffer)) ? (off_t)sizeof(buffer) : remaining;
                ssize_t bytes_read = read(arch_fd, buffer, chunk);
                if (bytes_read <= 0) break;
                ssize_t bytes_written = write(out_fd, buffer, bytes_read);
                if (bytes_written != bytes_read) {
                    perror("write extracted file");
                    close(out_fd);
                    close(arch_fd);
                    close(temp_fd);
                    exit(EXIT_FAILURE);
                }
                remaining -= bytes_read;
            }

            close(out_fd);
            chmod(entry.name, entry.mode);

            struct utimbuf times = { entry.mtime, entry.mtime };
            utime(entry.name, &times);

            printf("Extracted and removed: %s\n", entry.name);

            // делаем так, чтобы файл не добавлялся в новый архив
            if (remaining != 0)
                fprintf(stderr, "Warning: incomplete read for %s\n", entry.name);
        } else {
            // копируем в новый архив
            if (write(temp_fd, &entry, sizeof(entry)) != sizeof(entry)) {
                perror("write header");
                close(arch_fd);
                close(temp_fd);
                exit(EXIT_FAILURE);
            }

            off_t remaining = entry.size;
            while (remaining > 0) {
                ssize_t chunk = (remaining > (off_t)sizeof(buffer)) ? (off_t)sizeof(buffer) : remaining;
                ssize_t bytes_read = read(arch_fd, buffer, chunk);
                if (bytes_read <= 0) break;
                ssize_t bytes_written = write(temp_fd, buffer, bytes_read);
                if (bytes_written != bytes_read) {
                    perror("write temp");
                    close(arch_fd);
                    close(temp_fd);
                    exit(EXIT_FAILURE);
                }
                remaining -= bytes_read;
            }

            if (remaining != 0)
                fprintf(stderr, "Warning: truncated file %s in archive\n", entry.name);
        }
    }

    close(arch_fd);
    close(temp_fd);

    if (!found) {
        printf("File not found in archive: %s\n", target_name);
        unlink("tmp_archive.arc");
    } else {
        if (rename("tmp_archive.arc", archive_name) != 0)
            perror("rename temp archive");
    }
}

//  Вывод справки
void show_help() {
    printf("Usage:\n");
    printf("  ./archiver <archive> -i <file>   Add file to archive\n");
    printf("  ./archiver <archive> -e <file>   Extract and remove file from archive\n");
    printf("  ./archiver <archive> -s          Show archive contents\n");
    printf("  ./archiver -h                    Show this help\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        show_help();
        return 0;
    }

    if (argc < 3) {
        show_help();
        return 0;
    }

    const char *archive_name = argv[1];

    if (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--input") == 0) {
        if (argc < 4) {
            fprintf(stderr, "No file specified\n");
            return 1;
        }
        archive_add(archive_name, argv[3]);
    }
    else if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--extract") == 0) {
        if (argc < 4) {
            fprintf(stderr, "No file specified\n");
            return 1;
        }
        archive_extract(archive_name, argv[3]);
    }
    else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--stat") == 0) {
        archive_show(archive_name);
    }
    else {
        show_help();
    }

    return 0;
}

