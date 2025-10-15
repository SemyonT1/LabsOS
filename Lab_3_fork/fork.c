#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
void exit_handler() {
    printf("atexit handler, process %d quitting\n", getpid());
}

void sig_handler(int status) {
    fprintf(stderr, "sigint: sig handler with status %d\n", status);
    exit(1);
}

void term_handler(int status) {
    fprintf(stderr, "sigterm handler with status %d\n", status);
    exit(1);
}

int main() {
    atexit(exit_handler);

    pid_t pid;
    int status;

    struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
    sa.sa_handler = term_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTERM, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    signal(SIGINT, sig_handler);

    switch (pid=fork()) {
    case -1:
        perror("fork");
        exit(1);
    case 0:
        printf("CHILD: pid = %d, ppid = %d\n", getpid(), getppid());
        sleep(10);
        exit(3);
    default:
        wait(&status);
        printf("PARENT: pid = %d, ppid = %d\n", getpid(), getppid());
        if (WIFEXITED(status)) {
            printf("PARENT: child exited with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("PARENT: child killed by signal %d\n", WTERMSIG(status));
        }
        
        exit(0);
    }

    return 0;
}
