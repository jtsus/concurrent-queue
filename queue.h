//
// Created by Justin on 10/30/2022.
//

#ifndef ASGN2_QUEUE_H
#define ASGN2_QUEUE_H

#define DEBUG 0

#include <stdbool.h>
#include <stdint.h>

extern uint64_t push_attempts;
extern uint64_t pop_attempts ;

typedef struct queue queue_t;

queue_t *queue_new(int size);

void queue_delete(queue_t **q);

bool queue_push(queue_t *q, void *elem);

bool queue_pop(queue_t *q, void **elem);

#endif //ASGN2_QUEUE_H
