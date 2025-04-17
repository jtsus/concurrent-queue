//
// Created by Justin on 10/30/2022.
//

#include "queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

typedef struct Node {
    // Abstract data pointer representing contents of node
    void *value;
    // Forward pointer of this node, refers to the header when empty
    struct Node *forward;
    bool deleted;
    bool inserted;
} Node;

struct queue {
    Node *header;
    Node *tail;
    Node *deleted;
    int size;
    int max_size;

    pthread_mutex_t pop_lock;
    pthread_cond_t push_cond;

    pthread_mutex_t push_lock;
    pthread_cond_t pop_cond;
};

Node *node_new() {
    Node *N = (Node *) malloc(sizeof(Node));
    if (!N) return NULL;
    N->forward = NULL;
    N->deleted = false;
    N->value = NULL;
    N->inserted = false;
    return N;
}

queue_t *queue_new(int size) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    q->header = NULL;
    q->tail = NULL;
    q->deleted = node_new();
    q->size = 0;
    q->max_size = size;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    q->push_cond = cond;
    pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
    q->pop_cond = cond1;
    if (pthread_mutex_init(&q->pop_lock, NULL) != 0) {
        free(q);
        return NULL;
    }
    if (pthread_mutex_init(&q->push_lock, NULL) != 0) {
        free(q);
        return NULL;
    }
    return q;
}

void queue_delete(queue_t **q) {
    if (!q) return;
    queue_t *Q = *q;
    if (!Q) return;
    Node *N;
    while ((N = Q->header)) {
        // I guess why not make this thread safe too
        if (__sync_bool_compare_and_swap(&Q->header, N, N->forward)) {
            free(N);
        }
    }
    // This is allowed.
    while ((N = Q->deleted->forward)) {
        Q->deleted->forward = N->forward;
        free(N->value);
        free(N);
    }
    free(Q->deleted);
    //sem_destroy(&Q->pop_sem);
    //sem_destroy(&Q->push_sem);
    free(Q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {
    if (!q) return false;
    // Attempt to reserve space in the queue to insert into through the use of locks and conditional variables.
    pthread_mutex_lock(&q->push_lock);
    while (1) {
        if (q->size < q->max_size) {
            if (__sync_fetch_and_add(&q->size, 1) >= q->max_size) {
                __sync_fetch_and_sub(&q->size, 1); // should not be possible
            } else {
                break;
            }
        }
        if (DEBUG) {
            fprintf(stderr, "queue_t: waiting for pop signal\n");
        }
        pthread_cond_wait(&q->pop_cond, &q->push_lock);
    }
    pthread_mutex_unlock(&q->push_lock);
    // Once we have added to size we have guaranteed our spot in the queue, so we begin processing
    Node *N = node_new();
    // If the node failed to be initialized return false
    if (!N) return false;
    N->value = elem;
    // Replace current tail with the new node and get the old value. This is an atomic operation which ensures this
    // operation is the only one referencing this target.
    Node *target = __sync_lock_test_and_set(&q->tail, N);
    // Target is only null in the case nothing has been inserted yet, simply set inserted node as header.
    // Try to replace the target's forward reference with the new node only if it was previously null. The only case
    // where the forward is not null is if the node has been deleted (and the forward references itself). In this case
    // we should just set the header to be the inserted node.
    if (!target || !__sync_bool_compare_and_swap(&target->forward, NULL, N)) {
        q->header = N;
    }
    // Take effect point, signal waiting popping threads that there is work to be done
    pthread_cond_signal(&q->push_cond);
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    if (!q || elem == NULL) return false;
    // I found that the CAS-based implementation of pop that performs better than lock based is too complex to implement
    // in a short timeframe. This method uses a bulky lock with a conditional variable to get work done one by one.
    // However, it also employs the use of atomics to be easily synchronized with push.
    pthread_mutex_lock(&q->pop_lock);
    while (1) {
        Node *out = q->header;
        if (!out) {
            pthread_cond_wait(&q->push_cond, &q->pop_lock);
            continue;
        }
        // Set the forward reference of the out node to be itself and return the old next reference.
        // Since this field is also the only one referenced by queue_push we are sure to not run into any emerging
        // behavior like if a deleted boolean variable were to be used like in my last attempt.
        Node *next = __sync_lock_test_and_set(&out->forward, out);
        // Attempt to swap the header node to point to the next, we do not care if this succeeds since the only case
        // where the operation would fail is if a pushing thread had out as its target and found it was invalid and thus
        // pushed onto the header.
        __sync_bool_compare_and_swap(&q->header, out, next);
        // Take effect point
        __sync_fetch_and_sub(&q->size, 1);
        pthread_cond_signal(&q->pop_cond);
        pthread_mutex_unlock(&q->pop_lock);
        *elem = out->value;
        // Add out node to the list of nodes set to be deleted when the queue is freed. Of course this is far from
        // optimal, but it is a simple solution to the ABA problem that does not go against the spec. Note that this is
        // one of the times Java is actually the better language.
        Node *D = node_new();
        D->value = out;
        D->forward = __sync_lock_test_and_set(&q->deleted->forward, D);
        return true;
    }
}
