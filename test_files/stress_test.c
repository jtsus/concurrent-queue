//
// Created by Justin on 11/5/2022.
//

#include "queue.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define PUSH_CONCURRENCY 100
#define POP_CONCURRENCY 100
#define LOAD 10000

queue_t *Q;

void *fill_queue() {
    for (int i = 0; i < LOAD; i++) {
        queue_push(Q, NULL);
    }
    pthread_exit(NULL);
}

void *pop_queue(void *c) {
    int expected = PUSH_CONCURRENCY * LOAD;
    int remaining = expected / POP_CONCURRENCY;
    int *count = (int *) c;
    void *data = NULL;
    while (remaining > 0 && queue_pop(Q, (void **) &data)) {
        __sync_add_and_fetch(count, 1);
        remaining--;
    }
    pthread_exit(NULL);
}

int main() {
    int expected = PUSH_CONCURRENCY * LOAD;
    if ((expected / POP_CONCURRENCY) * POP_CONCURRENCY != expected) {
        printf("Invalid arguments\n");
        return 1;
    }
    Q = queue_new(PUSH_CONCURRENCY * LOAD / 2);
    pthread_t threads[PUSH_CONCURRENCY + POP_CONCURRENCY];
    int count = 0;
    for (int i = 0; i < PUSH_CONCURRENCY + POP_CONCURRENCY; i++) {
        pthread_create(&threads[i], NULL, i >= POP_CONCURRENCY ? fill_queue : pop_queue, &count);
    }
    //while (count < expected) {
    //    printf("Attempts: push: %lu, pop: %lu, %d/%d\n", push_attempts, pop_attempts, count, expected);
    //    sleep(1);
    //}
    for (int i = 0; i < PUSH_CONCURRENCY + POP_CONCURRENCY; i++) {
        pthread_join(threads[i], NULL);
    }
    queue_delete(&Q);
    if (count != expected) {
        printf("%d is not the correct amount\n", count);
        return 1;
    }
    return 0;
}
