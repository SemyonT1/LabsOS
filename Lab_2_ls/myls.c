#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define BLUE   "\033[34m"
#define GREEN  "\033[32m"
#define CYAN   "\033[36m"
#define RESET  "\033[0m"

int flag_l = 0, flag_a = 0;

typedef struct {
    char *name;
    struct stat st;
    char user[64];
    char group[64];
} FileInfo;

void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = S_ISDIR(mode) ? 'd' :
               S_ISLNK(mode) ? 'l' :
               S_ISCHR(mode) ? 'c' :
               S_ISBLK(mode) ? 'b' :
               S_ISSOCK(mode) ? 's' :
               S_ISFIFO(mode) ? 'p' : '-';
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
    printf("%s ", perms);
}

int cmpfunc(const void *a, const void *b) {
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    return strcmp(fa->name, fb->name);
}

void list_dir(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "myls: cannot access '%s': %s\n", path, strerror(errno));
        return;
    }

    struct dirent *entry;
    FileInfo *files = NULL;
    size_t count = 0;
    blkcnt_t total_blocks = 0;

    size_t w_nlink = 0, w_user = 0, w_group = 0, w_size = 0;

    // Сбор файлов и предварительный анализ
    while ((entry = readdir(dir))) {
        if (!flag_a && entry->d_name[0] == '.') continue;

        files = realloc(files, sizeof(FileInfo) * (count + 1));
        files[count].name = strdup(entry->d_name);

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &files[count].st) == -1) {
            perror("lstat");
            continue;
        }

        total_blocks += files[count].st.st_blocks;

        struct passwd *pw = getpwuid(files[count].st.st_uid);
        struct group  *gr = getgrgid(files[count].st.st_gid);

        if (pw) snprintf(files[count].user, sizeof(files[count].user), "%s", pw->pw_name);
        else snprintf(files[count].user, sizeof(files[count].user), "%d", files[count].st.st_uid);

        if (gr) snprintf(files[count].group, sizeof(files[count].group), "%s", gr->gr_name);
        else snprintf(files[count].group, sizeof(files[count].group), "%d", files[count].st.st_gid);
		// форматирование
        char buf[64];
        int len;
        len = snprintf(buf, sizeof(buf), "%lu", files[count].st.st_nlink);
        if ((size_t)len > w_nlink) w_nlink = len;

        len = strlen(files[count].user);
        if ((size_t)len > w_user) w_user = len;

        len = strlen(files[count].group);
        if ((size_t)len > w_group) w_group = len;

        len = snprintf(buf, sizeof(buf), "%ld", files[count].st.st_size);
        if ((size_t)len > w_size) w_size = len;

        count++;
    }
    closedir(dir);

    qsort(files, count, sizeof(FileInfo), cmpfunc);

    if (flag_l)
        printf("total %lld\n", (long long)total_blocks / 2); // st_blocks в 512 байтах, ls печатает в 1К

    // Вывод
    for (size_t i = 0; i < count; i++) {
        if (flag_l) {
            print_permissions(files[i].st.st_mode);
            printf("%*lu ", (int)w_nlink, files[i].st.st_nlink);
            printf("%-*s %-*s ", (int)w_user, files[i].user, (int)w_group, files[i].group);
            printf("%*ld ", (int)w_size, files[i].st.st_size);

            char timebuf[64];
            strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", localtime(&files[i].st.st_mtime));
            printf("%s ", timebuf);
        }

        // цветной вывод имени + стрелка для ссылок
        if (S_ISDIR(files[i].st.st_mode)) {
            printf(BLUE "%s" RESET, files[i].name);
        } else if (S_ISLNK(files[i].st.st_mode)) {
            printf(CYAN "%s" RESET, files[i].name);

            char target[1024];
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i].name);

            ssize_t len = readlink(fullpath, target, sizeof(target) - 1);
            if (len != -1) {
                target[len] = '\0';
                printf(" -> %s", target);
            }
        } else if (files[i].st.st_mode & S_IXUSR) {
            printf(GREEN "%s" RESET, files[i].name);
        } else {
            printf("%s", files[i].name);
        }

        printf("\n");
        free(files[i].name);
    }
    free(files);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        if (opt == 'l') flag_l = 1;
        else if (opt == 'a') flag_a = 1;
    }

    const char *path = ".";
    if (optind < argc) path = argv[optind];

    list_dir(path);
    return 0;
}

