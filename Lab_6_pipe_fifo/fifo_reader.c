#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

const char* myfifo = "myfifo";

struct tm get_time();

#define BUF_SIZE 1024

int main() {
    int fd = open(myfifo, O_RDONLY);

    char buf[BUF_SIZE];
    ssize_t read_n;

    struct tm time_info = get_time();
    printf("READER: \n\ttime = %s", asctime(&time_info));

    read_n = read(fd, buf, sizeof(buf));
    if (read_n == -1) {
        perror("read");
        exit(1);
    }
    printf("READER: has got message from writer:\n'%s'\n", buf);

    close(fd);

    return 0;
}

struct tm get_time() {
    time_t cur_time;
    time(&cur_time);
    struct tm time_info = *localtime(&cur_time);

    return time_info;
}