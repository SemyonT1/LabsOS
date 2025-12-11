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
    if (mkfifo(myfifo, 0644) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(1);
    }
    
    char buf[BUF_SIZE];
    
    struct tm time_info = get_time();
    sprintf(buf, "WRITER: \n\ttime = %s\tpid = %d\n", asctime(&time_info), getpid());

    printf("Sleep...\n");
    sleep(10);

    printf("Waiting for reader...\n");
    int fd = open(myfifo, O_WRONLY);

    if (fd == -1) {
        perror("open");
        unlink(myfifo);
        exit(1);
    }
    printf("Reader connected\n");


    printf("WRITER: sending message to a child\n");
    ssize_t written = write(fd, buf, strlen(buf) + 1);
    if (written == -1) {
        perror("write");
        unlink(myfifo);
        exit(1);
    }
    printf("WRITER has sent a message\n");

    close(fd);
    unlink(myfifo);
    
    return 0;
}

struct tm get_time() {
    time_t cur_time;
    time(&cur_time);
    struct tm time_info = *localtime(&cur_time);

    return time_info;
}
// 1
