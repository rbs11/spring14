#ifndef __TEST_DEFS_H_INCLUDED
#define __TEST_DEFS_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#define SCHED_IRUPT "Stopped by grader...\n"
#define SCHED_CRASH "** Crashed!\n"

void irupt(int);

#ifdef __BROKEN_INIT
#define gtthread_init(x) gtthread_init()
#endif

#ifdef __BROKEN_JOIN
#define gtthread_join(x, y) gtthread_join(&x, y)
#endif

#ifdef __BROKEN_EQUAL
#define gtthread_equal(x, y) gtthread_equal(&x, &y)
#endif

#ifdef __BROKEN_CANCEL
#define gtthread_cancel(a) gtthread_cancel(&a)
#endif

#ifdef __BROKEN_CREATE
#define gtthread_create(a, b, c) gtthread_create(a, NULL, b, c)
#endif

#ifdef __BROKEN_MUTEX_INIT
#define gtthread_mutex_init(a) gtthread_mutex_init(a, NULL) 
#endif

#ifdef __BROKEN_SELF
#define gtthread_self() *gtthread_self()
#endif

#ifdef __MISSING_SELF_PROTO
gtthread_t gtthread_self(void);
#endif

#ifdef __MISSING_EQUAL
#define gtthread_equal(x, y) (abort(), (gtthread_t) 0)
#endif

#ifdef __MISSING_YIELD
#define gtthread_yield() abort()
#endif

#ifdef __MISSING_TYPEDEF_1
typedef struct gtthread_t gtthread_t;
typedef struct gtthread_mutex_t gtthread_mutex_t;
#endif

#ifdef __NAMED_EVERYTHING_WRONG

typedef GTThread gtthread_t;
typedef GTThread_Mutex gtthread_mutex_t;

#define gtthread_create(a, b, c) GT_Create_Thread(a, b, c)
#define gtthread_join(a, b) GT_Join_Thread(a, b)
#define gtthread_exit(a) GT_Exit_Thread(a)
#define gtthread_yield() GT_Yield_Thread()
#define gtthread_self() GT_Self_Thread()
#define gtthread_equal(a, b) GT_Equal_Thread(a, b)
#define gtthread_cancel(a) GT_Cancel_Thread(a)

#define gtthread_mutex_init(a) GT_Initialize_Mutex(a)
#define gtthread_mutex_lock(a) GT_Lock_Mutex(a)
#define gtthread_mutex_unlock(a) GT_Unlock_Mutex(a)

#define gtthread_init(a) 
#endif

#ifdef __WRONG_SELF
#define gtthread_self() (abort(), (gtthread_t) {})
#endif

#ifdef __WRONG_CANCEL
#define gtthread_cancel(a) (abort(), (gtthread_t) {})
#endif

#ifdef __REQUIRES_BEGIN
#define gtthread_create(a, b, c) gtthread_create(a, b, c); gtthread_begin(a)
#endif

// #define main() main2()

#endif
