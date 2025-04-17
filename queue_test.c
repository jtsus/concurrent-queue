#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

#define QUEUE_MAX 500

int test_insert(queue_t *Q) {
    int i = 2;
    if (!queue_push(Q, &i)) return 1;
    int v = 3;
    int *b = &v;
    if (!queue_pop(Q, (void **) &b)) return 2;
    if (*b != i) return 3;
    return 0;
}

void *fill_random(void *Q) {
    for (int i = 0; i < 100; i++) {
        queue_push((queue_t *)Q, NULL);
    }
    pthread_exit(NULL);
}

int test_order(queue_t *Q) {
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, fill_random, Q);
    }
    int count = 0;
    void *data = NULL;
    while (count < 400 && queue_pop(Q, &data)) {
        count++;
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return count == 400 ? 0 : (count + 1);
}

//void pop_all(queue_t *Q, int *count) {
//    void *data = NULL;
//    while (queue_pop(Q, &data)) {
//        __sync_fetch_and_add(count, 1);
//    }
//}
//
//int test_conc_pop(queue_t *Q) {
//    for (int i = 0; i < 4; i++) {
//        fill_random(Q);
//    }
//    int count = 0;
//    pthread_t threads[4];
//    for (int i = 0; i < 4; i++) {
//        pthread_create(&threads[i], NULL, pop_all, Q, &count);
//    }
//    for (int i = 0; i < 4; i++) {
//        void *out = NULL;
//        pthread_join(threads[i], &out);
//    }
//    int expected = 400;
//    if (expected < QUEUE_MAX) expected = QUEUE_MAX;
//    return count == expected;
//}

int main() {
    time_t t;
    srand((unsigned) time(&t));
    int (*tests[100])(queue_t *);
    int i = 0;
    tests[i++] = test_insert;
    tests[i++] = test_order;
    //tests[i++] = test_conc_pop;
    int result = 0;
    for (int j = 0; j < i; j++) {
        printf("running test %d: ", j + 1);
        queue_t *Q = queue_new(QUEUE_MAX);
        int out = tests[j](Q);
        queue_delete(&Q);
        printf("%d\n", out);
        if (result == 0) result = out;
    }
    return result;
}
