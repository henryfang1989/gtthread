#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>

#include "philosopher.h"
pthread_mutex_t stick[5];

/*
 * Performs necessary initialization of mutexes.
 */
void chopsticks_init(){
    pthread_mutex_init(&stick[0], NULL);
    pthread_mutex_init(&stick[1], NULL);
    pthread_mutex_init(&stick[2], NULL);
    pthread_mutex_init(&stick[3], NULL);
    pthread_mutex_init(&stick[4], NULL);
}

/*
 * Cleans up mutex resources.
 */
void chopsticks_destroy(){
    pthread_mutex_destroy(&stick[0]);
    pthread_mutex_destroy(&stick[1]);
    pthread_mutex_destroy(&stick[2]);
    pthread_mutex_destroy(&stick[3]);
    pthread_mutex_destroy(&stick[4]);
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void pickup_chopsticks(int phil_id){
        printf("philosopher %d is hungry\n", phil_id);
    if(phil_id == 4){
            pthread_mutex_lock(&stick[0]);
            pickup_right_chopstick(phil_id);
    }
    else{
            pthread_mutex_lock(&stick[phil_id]);
            pickup_left_chopstick(phil_id);
    }
        printf("philosopher %d gets the first chopstick\n", phil_id);
    if(phil_id == 4){
        pthread_mutex_lock(&stick[4]);
        pickup_left_chopstick(phil_id);
    }
    else{
        pthread_mutex_lock(&stick[phil_id+1]);
        pickup_right_chopstick(phil_id);
    }
        printf("philosopher %d gets the second chopstick\n", phil_id);
        printf("philosopher %d is eating\n", phil_id);
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void putdown_chopsticks(int phil_id){
        //get chopsticks
        printf("philosopher %d finishes eating\n", phil_id);
        putdown_right_chopstick(phil_id);
        if(phil_id == 4)
            pthread_mutex_unlock(&stick[0]);
        else
            pthread_mutex_unlock(&stick[phil_id+1]);
        printf("philosopher %d puts down second chopstick\n", phil_id);
        putdown_left_chopstick(phil_id);
        pthread_mutex_unlock(&stick[phil_id]);
        printf("philosopher %d puts down first chopstick\n", phil_id);
        printf("philosopher %d is thinking\n", phil_id);
}