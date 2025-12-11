#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>

#define BUF_SIZE 1024

struct tm get_time();

int main() {

    int pipefds[2]; // 0 - чтение, 1 - запись
    if (pipe(pipefds) == -1) {
        perror("pipe");
        exit(1);
    }

    char buf[BUF_SIZE];

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) { // дочерний процесс
        close(pipefds[1]);

        // чтение процесса
        int read_n = read(pipefds[0], buf, sizeof(buf));
        if (read_n == -1) {
            perror("read");
            exit(1);
        }
        
        struct tm time_info = get_time();

        printf("CHILD: \n\ttime = %s\tpid = %d\n", asctime(&time_info), getpid());
        printf("CHILD: has got message from parent:\n'%s'\n", buf);

        close(pipefds[0]);
    }
    else { // родитель
        close(pipefds[0]);

        // записываем процесс        
        struct tm time_info = get_time();

        sprintf(buf, "PARENT: \n\ttime = %s\tpid = %d\n", asctime(&time_info), getpid());
        printf("%s", buf);

        printf("Sleep...\n");
        sleep(5);
        
        printf("PARENT: sending message to a child\n");
        int written = write(pipefds[1], buf, strlen(buf) + 1);
        if (written == -1) {
            perror("write");
            exit(1);
        }
        
        close(pipefds[1]);
    }

    return 0;
}

struct tm get_time() {
    time_t cur_time;
    time(&cur_time);
    struct tm time_info = *localtime(&cur_time);

    return time_info;
}
