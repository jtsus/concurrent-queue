//
// Created by Justin on 11/5/2022.
//

#include "queue.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define PUSH_CONCURRENCY 100
#define POP_CONCURRENCY 100
#define LOAD 10000

queue_t *Q;

void *fill_queue(void *input) {
    int id = *((int *) input);
    for (int i = 1; i <= LOAD; i++) {
        int *memory = malloc(sizeof(int));
        *memory = (id << 16) & i;
        queue_push(Q, memory);
    }
    pthread_exit(NULL);
}

void *pop_queue(void *c) {
    int expected = PUSH_CONCURRENCY * LOAD;
    int remaining = expected / POP_CONCURRENCY;
    int *count = (int *) c;
    int *temp = NULL;
    int highest_values[PUSH_CONCURRENCY] = { 0 };
    while (remaining > 0 && queue_pop(Q, (void **) &temp)) {
        __sync_add_and_fetch(count, 1);
        remaining--;
        int data = *temp;
        int thread = data >> 16;
        int value = data & 0xffff;
        if (value < highest_values[thread]) {
            printf("Invalid ordering detected! %d vs %d\n", highest_values[thread], value);
            exit(1);
        }
        highest_values[thread] = value;
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
        if (i >= POP_CONCURRENCY) {
            int *memory = malloc(sizeof(int));
            *memory = i;
            pthread_create(&threads[i], NULL, fill_queue, memory);
        } else {
            pthread_create(&threads[i], NULL, pop_queue, &count);
        }
    }
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
