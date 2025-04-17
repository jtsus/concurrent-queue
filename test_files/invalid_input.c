//
// Created by Justin on 11/5/2022.
//

#include "queue.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    queue_t *Q = queue_new(100);
    if (queue_pop(Q, NULL)) {
        printf("Allowed NULL pop element pointer\n");
        return 1;
    }
    if (queue_pop(NULL, (void **) &Q)) {
        printf("Allowed NULL pop pointer\n");
        return 1;
    }
    if (queue_push(NULL, Q)) {
        printf("Allowed NULL push pointer\n");
        return 1;
    }
    queue_delete(&Q);
    return 0;
}
