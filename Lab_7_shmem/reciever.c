#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#define SHM_SIZE 512

const char* shm_name = "SHM_FILE";
char* shm = NULL;

void cleanup(int sig) {
    if (shm != NULL) {
        shmdt(shm);
    }
    _exit(0);
}
int main() {
    signal(SIGINT, cleanup);
    int fd = open(shm_name, O_CREAT | O_RDONLY, 0600);
    if (fd == -1) {
        printf("Shared memory ещё не создана (запустите sender)\n");
        return 1;
    }
    close(fd);

    
    key_t key = ftok(shm_name, 2);
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    
    shm = shmat(shmid, NULL, 0);
    if (shm == (char *)-1) {
        perror("shmat");
        return 1;
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str),
             "%Y-%m-%d %H:%M:%S", tm_info);

    printf("Receiver PID=%d, время=%s\n", getpid(), time_str);
    printf("Получено: %s\n", shm);

    if (shmdt(shm) == -1 ) {
        perror("shmdt");
        return 1;
    }

    return 0;
}
