# include<stdio.h>
# include<unistd.h>
# include<pthread.h>
# include"atomic.h"
#define THREAD_COUNT 10

void* thread_callback(void *arg){
    int * pcount = (int *)arg;  
    int i = 0;

    while(i++ < 100000){
        // (*pcount)++;
        add(pcount,1);
        usleep(1);
    }
}

int main (){
    pthread_t threadid[THREAD_COUNT] = {0}; // 初始化
    int i = 0;
    int count = 0;

    // 创建线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_callback, &count);// thread_callback, &count : 函数与其参数
    }

    for (i = 0; i < 10; i++){
        printf("count : %d\n", count);
        sleep(1);
    }
    return 0;
}