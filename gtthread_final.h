#ifndef GTTHREAD_H
#define GTTHREAD_H

#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/time.h>
#include"steque.h"


/* Define gtthread_t and gtthread_mutex_t types here */
typedef unsigned int gtthread_t;
#define MAX_THREADS 3000

//context node status
#define VALID 0
#define RUNNING 1
#define FINISHED 2
#define CANCELED 3

typedef struct {
	volatile int *value;
} gtthread_mutex_t;

typedef struct context_node_t{
    ucontext_t new_context; 
    void *ret;
    unsigned int valid;
    unsigned int id;
} context_node_t;


long interval;
steque_t threads;
context_node_t *current;
context_node_t nodes_pool[MAX_THREADS];


void signal_unblock(void);
void signal_block(void);
void set_timer(void);
void signal_handler(int);
void change_thread(void);
void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);
#endif
