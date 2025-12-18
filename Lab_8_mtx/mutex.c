#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_LEN 512
#define READERS 10
#define MAX_WRITES 20

pthread_mutex_t lock;

char shared_buf[BUF_LEN];
int write_counter = 0;
int finished = 0;

void* reader_thread(void* arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&lock);

        if (finished) {
            pthread_mutex_unlock(&lock);
            break;
        }

        printf("[READER %lu] buffer: %s\n",
               (unsigned long)pthread_self(),
               shared_buf);
        
        pthread_mutex_unlock(&lock);
        usleep(200000);
    }

    return NULL;
}

void* writer_thread(void* arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&lock);

        if (write_counter >= MAX_WRITES) {
            finished = 1;
            pthread_mutex_unlock(&lock);
            break;
        }

        snprintf(shared_buf, BUF_LEN,
                 "write #%d", write_counter);

        write_counter++;

        pthread_mutex_unlock(&lock);
        usleep(300000);
    }

    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[READERS];

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "mutex init error\n");
        return 1;
    }

    pthread_create(&writer, NULL, writer_thread, NULL);

    for (int i = 0; i < READERS; ++i) {
        pthread_create(&readers[i], NULL, reader_thread, NULL);
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < READERS; ++i) {
        pthread_join(readers[i], NULL);
    }

    return 0;
}
