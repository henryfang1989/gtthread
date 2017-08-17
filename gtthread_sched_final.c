/**********************************************************************
  gtthread_sched.c.  

  This file contains the implementation of the scheduling subset of the
  gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
   Include as needed
 */

#include "gtthread.h"

/* 
   Students should define global variables and helper functions as
   they see fit.
 */

void *get_context_node(ucontext_t context, gtthread_t *thread){
	int i;
	printf("try to find a valid node from nodes pool\n");
	for(i = 0; i < MAX_THREADS; i++){
		if(nodes_pool[i].valid == VALID)
		{
			nodes_pool[i].id = i;
			*thread = i;
			printf("get a valid # %d node from nodes pool\n", i);
			nodes_pool[i].valid = RUNNING;
			printf("add new create context to node\n");
			nodes_pool[i].new_context = context;
			return (void*)(&nodes_pool[i]); 
		}
	}
	perror("nodes pool: Out of threads\n");
	return (void*)NULL;
}

unsigned int check_status(thread){
	return nodes_pool[thread].valid;
}

void signal_block(void){
	sigset_t vtalrm;
	sigemptyset(&vtalrm);
	sigaddset(&vtalrm, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &vtalrm, NULL);
}

void signal_unblock(void){
	sigset_t vtalrm;
	sigemptyset(&vtalrm);
	sigaddset(&vtalrm, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
} 

void set_timer(void){
printf("start to set a timer\n");
	struct itimerval *T;
	struct sigaction act, oact;

	signal_block();

    printf("assign entry func to handler\n");
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGVTALRM, &act, &oact);

	printf("set period for timer\n");
	T = (struct itimerval*) malloc(sizeof(struct itimerval));
	T->it_value.tv_sec = T->it_interval.tv_sec = 0;
	T->it_value.tv_usec = T->it_interval.tv_usec = interval;

	if(0 != setitimer(ITIMER_VIRTUAL, T, NULL)){
		printf("fail to set up a alarm\n");
	}

	printf("start the timer\n");
	signal_unblock();
}


//save return value in gtthread_exit()
void save_retval(void *retval) {
	int index;
	index = current->id;
	printf("save # %d node's return value\n", index);
	nodes_pool[index].ret = retval;
}


//start to excute the start routine
void thread_run(void *(*start_routine)(void *), void *arg) {

		void *retval = start_routine(arg);
		gtthread_exit(retval);

}

void next_valid_node(void){
	while(current->valid != RUNNING){
		steque_pop(&threads);
		current = (context_node_t*)steque_front(&threads);
	}
}

void signal_handler(int sig){
	change_thread();
}

void change_thread(void){
	printf("Change thread\n");
//`	signal_block();
	context_node_t *pre;
	pre = (context_node_t*)steque_front(&threads);

	//thread has not finished
	if(check_status(pre->id) == RUNNING){
		printf("the# %d  thread is running\n", pre->id);
		printf("put # %d thread to the end of queue\n", pre->id);
		steque_cycle(&threads);
		current = (context_node_t*)steque_front(&threads);
		printf("find the next valid # %d node in queue\n", current->id);
		next_valid_node();
		if(current->id != pre->id){
			printf("start the timer for the new thread\n");
			set_timer();
			printf("swap the next valid # %d thread with previous # %d thread\n", current->id, pre->id);
			swapcontext(&(pre->new_context), &(current->new_context));
		}
		else{
			printf("no other threads exist, continue in the same thread\n");
			printf("start a timer\n");
			set_timer();
			return;
		}
	}
	//thread finished by exit or cacnel
	else if(check_status(pre->id) != RUNNING) {
		printf("the # %d thread is finished or canceled\n", pre->id);
		printf("pop out the # # %d finished or canceled thread in queue\n", pre->id);
		steque_pop(&threads);
		current = (context_node_t*)steque_front(&threads);
		printf("find the next valid # %d  node in queue\n", current->id);
		next_valid_node();
		printf("start the timer for the new thread\n");
		set_timer();
		if(current != NULL){
			printf("swap the next valid # %d thread with previous # %d thread\n", current->id, pre->id);
			swapcontext(&(pre->new_context), &(current->new_context));
		}
		else
			return;
	}
	//signal_unblock();
}

/*
   The gtthread_init() function does not have a corresponding pthread equivalent.
   It must be called from the main thread before any other GTThreads
   functions are called. It allows the caller to specify the scheduling
   period (quantum in micro second), and may also perform any other
   necessary initialization.  If period is zero, then thread switching should
   occur only on calls to gtthread_yield().

   Recall that the initial thread of the program (i.e. the one running
   main() ) is a thread like any other. It should have a
   gtthread_t that clients can retrieve by calling gtthread_self()
   from the initial thread, and they should be able to specify it as an
   argument to other GTThreads functions. The only difference in the
   initial thread is how it behaves when it executes a return
   instruction. You can find details on this difference in the man page
   for pthread_create.
 */
void gtthread_init(long period){
	printf("init the queue and nodes pool\n");
	steque_init(&threads);
	interval = period;
	memset(nodes_pool, 0, MAX_THREADS*sizeof(context_node_t));

	printf("create the first context\n");
	ucontext_t context;
	gtthread_t first_thread_id;
	getcontext(&context);
	printf("add the new node into queue\n");
	printf("get a valid node from nodes pool\n");
	steque_enqueue(&threads, get_context_node(context, &first_thread_id));
	current = (context_node_t*)steque_front(&threads);
	printf("start the timer\n");
	set_timer(); 
}

/*
   The gtthread_create() function mirrors the pthread_create() function,
   only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
		void *(*start_routine)(void *),
		void *arg){

	//create a new context
	printf("create a new context\n");
	ucontext_t context;
	if(0 != getcontext(&context)){
		perror("getcontext");
		exit(EXIT_FAILURE);
	}
	context.uc_stack.ss_sp = (char*)calloc(1, SIGSTKSZ);
	context.uc_stack.ss_size = SIGSTKSZ;
	//setup the new context
	printf("add entery func for the new context\n");
	makecontext(&context, thread_run, 2, start_routine, arg);
	//add it into queue
	//signal_block();
	printf("add new node to queue\n");
	steque_enqueue(&threads, get_context_node(context, thread));
	//signal_unblock();
	return 0;
}

/*
   The gtthread_join() function is analogous to pthread_join.
   All gtthreads are joinable.
 */
int gtthread_join(gtthread_t thread, void **status){
	
	if(current->id == thread){
		return -1;
	}
	//join thread is till rinning
	while(RUNNING == check_status(thread)){
		printf("joined thread # %d is till running\n", thread);
		gtthread_yield();
	}
	//join thread is finished or canceled
	if(FINISHED == check_status(thread) || 
			CANCELED == check_status(thread)){
		printf("release the return value for joined thread # %d\n", thread);
		//signal_block();
		if(status != NULL){
			*status = nodes_pool[thread].ret;
		}
		nodes_pool[thread].valid = VALID;
		//signal_unblock();
		return 0;
	}
	//thread is not exist or not be used
	else
		return -1;
}

/*
   The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* retval){
	//signal_block();
	save_retval(retval);
//	nodes_pool[((context_node_t*)steque_front(&threads))->id].valid = FINISHED;
	printf("# %d node is finished\n", current->id);
	current->valid = FINISHED;
	change_thread();
	//signal_unblock();
}


/*
   The gtthread_yield() function is analogous to pthread_yield, causing
   the calling thread to relinquish the cpu and place itself at the
   back of the schedule queue.
 */
void gtthread_yield(void){
	printf("start yield\n");
	printf("# %d node yield\n", current->id);
	change_thread();
}

/*
   The gtthread_yield() function is analogous to pthread_equal,
   returning zero if the threads are the same and non-zero otherwise.
 */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
	printf("t1:%d, t2:%d\n", t1, t2);
	if(t1 == t2){
		printf("%d is equal to %d\n", t1, t2);
		return 1;
	}
		
	else {
		printf("%d is not equal to %d\n", t1, t2);
		return 0;
	}		
}

/*
   The gtthread_cancel() function is analogous to pthread_cancel,
   allowing one thread to terminate another asynchronously.
 */
int  gtthread_cancel(gtthread_t thread){
	printf("thread # %d is canceled\n", thread);
	//signal_block();
	if(check_status(thread) != RUNNING){
		return -1;
	}

	int *ret_value;
	ret_value = calloc(1, sizeof(int));
	*ret_value = CANCELED;
	nodes_pool[thread].valid = CANCELED;

	//ensure join thread can get return value
	nodes_pool[thread].ret = ret_value;

	if(current->id == thread){
		change_thread();
	}
	//signal_unblock();
	return 0;
}

/*
   Returns calling thread.
 */
gtthread_t gtthread_self(void){
printf("self is: %d\n", current->id);
	return current->id;
}
