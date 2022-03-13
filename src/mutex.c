#include "mutex.h"
#include "fail.h"
#include <stdio.h>

mutex_ref mutex_alloc(void);
MEMORY_ALLOC_DEF_DECL(mutex);
MEMORY_ALLOC_DEF_DECL(mutex_operation);

hash_t
pthread_mutex_hash(pthread_mutex_t *m)
{
    return (hash_t)m;
}

bool
mutexes_equal(mutex_refc m1, mutex_refc m2)
{
    if (!m1) return m2 == NULL;
    if (!m2) return m1 == NULL;
    return m1->mutex == m2->mutex;
}

mutex_ref
mutex_copy(mutex_refc refc)
{
    if (!refc) return NULL;
    mutex_ref cpy = mutex_alloc();
    if (!cpy) return NULL;
    cpy->mutex = refc;
    cpy->owner = refc->owner;
    cpy->state = refc->state;
    return cpy;
}

void
mutex_destroy(mutex_ref ref)
{
    if (!ref) return;
    free(ref);
}

bool
pthread_mutexes_equal(pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    // TODO: A source program that uses the same
    // address to store the pthread_mutex_t opaque
    // struct for multiple mutexes, while confusing,
    // would break equality. Ideally, the identity should
    // be based on the values of the struct or on another
    // layer of indirection that assigns, for each MUTEX_INIT
    // of the same pthread_mutex_t addr a unique identifier which
    // could be used to identify a particular mutex.
    if (!m1) return m2 == NULL;
    if (!m2) return m1 == NULL;
    return m1 == m2;
}

bool
mutex_operations_race(mutex_operation_refc mutop1, mutex_operation_refc mutop2)
{
    if (!mutop1 || !mutop2) return NULL;
    return mutexes_equal(&mutop1->mutex, &mutop2->mutex);
}

bool
mutex_operation_enabled(mutex_operation_refc mop, thread_ref thread)
{
    if (!mop || !thread) return false;

    switch (mop->type) {
        case MUTEX_LOCK:
            return mop->mutex.state != MUTEX_LOCKED;
        default:
            return true;
    }
}

bool
mutex_operation_creates_mutex(mutex_operation_refc op, mutex_refc mut)
{
    if (!op || !mut) return false;
    if (op->type != MUTEX_INIT) return false;
    return mutexes_equal(&op->mutex, mut);
}

bool
mutex_operations_coenabled(mutex_operation_refc op1, mutex_operation_refc op2)
{
    if (!op1 || !op2) return false;

    if (mutex_operation_creates_mutex(op1, &op2->mutex)
    || mutex_operation_creates_mutex(op2, &op1->mutex))
        return false;

    if (op1->type == MUTEX_LOCK && op2->type == MUTEX_LOCK)
        return !mutexes_equal(&op1->mutex, &op2->mutex);

    return true;
}

PRETTY_PRINT_DEF_DECL(mutex);
PRETTY_PRINT_STATE_DEF_DECL(mutex);

void
mutex_pretty_off(mutex_refc mut, unsigned int off)
{
    space(off); puts("**** MUTEX ****");
    space(off); printf(" pthread_mutex: %p \n", mut->mutex);
    space(off); printf(" owner: %p \n", mut->owner);
    mutex_state_pretty_off(mut->state, off);
    space(off); puts("************");
}

void
mutex_state_pretty_off(mutex_state state, unsigned int off)
{
    space(off);
    switch (state) {
        case MUTEX_UNKNOWN:     puts("MUTEX_UNKNOWN"); break;
        case MUTEX_UNLOCKED:    puts("MUTEX_UNLOCKED"); break;
        case MUTEX_LOCKED:      puts("MUTEX_LOCKED"); break;
        case MUTEX_DESTROYED:   puts("MUTEX_DESTROYED"); break;
        default:                mc_unimplemented();
    }
}