#ifndef _SAFE_QUEUE_
#define _SAFE_QUEUE_
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <queue>
#include <atomic>
using namespace std;
template<typename element_type>
class safe_queue{
public:
    // 构造与析构函数
    safe_queue(){
        que = new queue<element_type>();
    }
    ~safe_queue(){
        delete que;
    }

    // 功能函数
    element_type front(){
        pthread_mutex_lock(&mutex);
        // 如若队列为空,则等待
        if (que->empty()){
            pthread_cond_wait(&queue_is_empty, &mutex);
        }
        element_type res = que->front();
        pthread_mutex_unlock(&mutex);
        return res;
    }
    element_type pop(){
        pthread_mutex_lock(&mutex);
        // 如若队列为空,则等待
        // 慢速系统调用会被中断打断
        while (que->empty()){
            // wait queue
            // 将自己的TCB指针挂载等待队列尾部 
            // 将自己状态设置为 可中断睡眠态
            // 调用 schedule() 
            pthread_cond_wait(&queue_is_empty, &mutex);
            // 未决,信号屏蔽 PCB
            // do_signal()
            // 每次发生中断(时钟),进入内核态,处理中断。处理信号PCB。
            // iret, cs,ip;
            // sys_return 
            // 继续do_signal();
            // 为什么要进入到用户态? 
            // 用户态。
        }
        element_type res = que->front();
        que->pop();
        pthread_mutex_unlock(&mutex);
        return res;
    }

    void push(element_type element){
        pthread_mutex_lock(&mutex);
        que->push(element);
        pthread_mutex_unlock(&mutex);
        // 找到这个结构体内部的等待队列
        // 将这个等待中的TCB取出。
        // 将第一个元素设置为就绪态。并且将其放入就绪队列(内核的全局变量)中。
        pthread_cond_signal(&queue_is_empty);
    }

    bool empty(){
        pthread_mutex_lock(&mutex);
        bool res = que->empty();
        pthread_mutex_unlock(&mutex);
        return res;
    }

private :
    queue<element_type>* que;
    pthread_mutex_t mutex;
    pthread_cond_t queue_is_empty;   // 队列是空的时候锁上
};
#endif