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

int shmid = -1;
const char* shm_name = "SHM_FILE";
char* shm = NULL;
void cleanup(int sig) {
    if (shm != NULL) {
        shmdt(shm);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    unlink(shm_name);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    int fd = open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        if (errno == EEXIST) {
            printf("Sender уже запущен\n");
        } else {
            perror("open");
        }
        return 1;
    }
    close(fd);

    key_t key = ftok(shm_name, 2);
    if (key == -1) {
        perror("ftok");
        cleanup(0);
    }

    
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget");
        cleanup(0);
    }

    shm = shmat(shmid, NULL, 0);
    if (shm == (char *)-1) {
        perror("shmat");
        cleanup(0);
    }

    printf("Sender запущен. PID = %d\n", getpid());

    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        char time_str[64];
        strftime(time_str, sizeof(time_str),
                 "%Y-%m-%d %H:%M:%S", tm_info);

        snprintf(shm, SHM_SIZE,
                 "Отправитель PID=%d, время=%s",
                 getpid(), time_str);

        sleep(1);
    }
    cleanup(0);

    return 0;
}
