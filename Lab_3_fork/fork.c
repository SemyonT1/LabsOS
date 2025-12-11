#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <bits/sigaction.h>

void exit_handler() {
    printf("atexit handler\n");
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
        printf("PARENT: child exit status = %d\n", WEXITSTATUS(status));
        
        exit(0);
    }

    return 0;
}
