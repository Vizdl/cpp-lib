# include<stdio.h>
# include<unistd.h>
# include<pthread.h>
# include"atomic.h"

int counter = 0;
#define TIMES 20000
#define THREAD_COUNT 10
pthread_t threadid[THREAD_COUNT] = {0};

void* thread_callback(void *arg){
    int i = 0;
    while(i++ < TIMES){
        add(&counter,1);
        usleep(1);
    }
}

int main (){
    int i = 0;
    void *status;

    // 创建线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_callback, NULL);
    }
    // 等待线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }

    if (counter != TIMES * THREAD_COUNT)
        printf("error!!!\n");
    else
        printf("yes!!!\n");
    
    return 0;
}