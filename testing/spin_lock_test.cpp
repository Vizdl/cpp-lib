#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include "spin_lock.h"

struct spinlock lock;
int counter = 0;
#define TIMES 2000000
#define THREAD_COUNT 10
pthread_t threadid[THREAD_COUNT] = {0};

void* thread_callback(void *arg){
    int i = 0;
    while(i++ < TIMES){
        spin_lock(&lock);
        counter++;
        spin_unlock(&lock);
    }
}

int main (){
    int i = 0;
    void *status;

    // 初始化 spinlock
    init_lock(&lock);

    // 创建线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_callback, NULL);
    }

    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }

    if (counter != TIMES * THREAD_COUNT)
        printf("error!!!\n");
    else
        printf("yes!!!\n");
    
    return 0;
}