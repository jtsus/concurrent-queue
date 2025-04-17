# Concurrent Bounded Queue

An implementation of a semi lock-free (on push) concurrent bounded queue using linked nodes and atomic operations.

This implementation allows an unbounded number of producers and an unbounded number of consumers.

## Guarantees

This queue is operated under the following guarantees:
1. Any operation invoked under valid conditions will return true.
2. On or during completion of an operation the queue will always be in a valid state.
    - Valid in this context means that none of the guarantees of the queue are broken
3. Each operation has a take effect point that is before the return of the function
4. The queue will never pop the same element twice
5. The queue will never pop an element before it has been completely pushed
6. The queue will never push an element onto an element being popped
8. The queue will never exceed its given capacity

## Implementation

When an operation is in contention, the queue allows both to fight to take hold of a specific context to use to complete
the operation. Multiple threads may (and likely will) try processing the same operation under the same context, this is
where atomic operations are used ensure only one thread claims the right to process the operation under that
context. Once a context is claimed by a thread the other thread immediately switches to the next context to attempt the
operation again.

Originally the implementation was planned to completely disregard the use of locks entirely, however they are needed in
order to safely implement the blocking if empty/full. I also deemed that the complexity required to implement a lock
free and wait free implementation of pop is too high to be done in this timeline, so I also used locks for pop.
