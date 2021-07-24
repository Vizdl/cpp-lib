#ifndef _THREAD_POOL_THREAD_
#define _THREAD_POOL_THREAD_

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
#include "thread_pool_task.h"
#include "safe_queue.h"
using namespace std;
/*
线程节点。
线程的几种状态分析 :
NOT_START : 只创建,未start
ALIVE : 正在运行
WAIT_TASKQUEUE : 等待该线程的任务队列清空才退出,也有可能会堵塞在任务队列上。这个时候需要外力来退出。
DESTROY : 如若正在执行任务,则任务结束后退出,否则直接退出,也有可能会堵塞在任务队列上。这个时候需要外力来退出。
*/
template<typename parameter_name, typename result_name>
class thread{
public:
    #define DESTROY 2  // DESTROY在执行完当前任务(如若有的话)就直接退出。
    #define ALIVE 1     
    #define WAIT_TASKQUEUE 3 // WAIT_TASKQUEUE是等到任务队列为空才能退出。
    #define NOT_START 0
    #define SLEEP 4 // 线程休眠中。

    // 构造和析构
    thread(safe_queue<task_t<parameter_name, result_name>*>* sq)
    : sq(sq){
        pthread_spin_lock(&status_lock);
        thread_status = NOT_START;
        pthread_spin_unlock(&status_lock);

        pthread_create(&tid, NULL, thread_call_back, this);
        // 使得线程成为游离态
        pthread_detach(tid);
    }

    bool thread_is_sleep(){
        pthread_spin_lock(&status_lock);
        bool is_sleep = (thread_status == SLEEP);
        pthread_spin_unlock(&status_lock);
        return is_sleep;
    }

    // 设置线程死亡
    void wait_to_die(){
        pthread_spin_lock(&status_lock);
        thread_status = WAIT_TASKQUEUE;
        pthread_spin_unlock(&status_lock);
    }

    void destroy(){
        pthread_spin_lock(&status_lock);
        thread_status = DESTROY;
        pthread_spin_unlock(&status_lock);
    }
    
    pthread_t gettid(){
        pthread_spin_lock(&status_lock);
        if (thread_status == NOT_START){
            perror("线程未start");
        }
        if (thread_status == DESTROY ||
            thread_status == WAIT_TASKQUEUE ){
            perror("线程已死亡");
        }
        pthread_spin_unlock(&status_lock);
        return tid;
    }

private:
    // 该函数的参数为 一个thread对象,这样就能实现一个静态方法却能访问不同的对象。
    static void* thread_call_back(void* arg){
        thread<parameter_name, result_name>* tptr = (thread<parameter_name, result_name>*)arg;
        // tptr->tid = std::this_thread::get_id();

        // 当开始回调才证明线程活着
        pthread_spin_lock(&tptr->status_lock);
        tptr->thread_status = ALIVE;
        pthread_spin_unlock(&tptr->status_lock);
        
        // 判断这个线程的状态是否活着 
        // atomic_read(&tptr->thread_status) == ALIVE
        while (true){

            // 如若是销毁的话,那么就直接死亡。
            // 如若队列为空且又是延时死亡的话就退出。
            pthread_spin_lock(&tptr->thread_status);
            if (tptr->thread_status == DESTROY ||
             (tptr->sq->empty() && tptr->thread_status == WAIT_TASKQUEUE) ){
                pthread_spin_unlock(&tptr->status_lock);
                break;
            }// 设置线程为休眠态
            tptr->thread_status = SLEEP; 
            pthread_spin_unlock(&tptr->status_lock);

            // 线程很有可能堵塞在这里,而队列又恰好为空,无法销毁。
            // 而会堵塞在这意味着,任务队列一定为空,除了ALIVE态其他的都可以被信号杀死。
            task_t<parameter_name, result_name>* task = tptr->sq->pop();


            // 如若线程为休眠态
            pthread_spin_lock(&tptr->status_lock);
            if (SLEEP == tptr->thread_status){
                tptr->thread_status = ALIVE;
            }
            pthread_spin_unlock(&tptr->status_lock);

            // 要保证 task 存在
            if (task == NULL){
                // printf("第几次进入 %d task is NULL\n", i);
                perror("task is NULL");
            }
            result_name res = task->get_call_back_ptr()(task->get_arg()); // 调用任务
            task->set_result(res); // 设置结果
        }
        return NULL;
    }


    typedef result_name(*call_back_ptr)(parameter_name); // 声明函数指针
    int thread_status; // 线程是否活着
    pthread_t tid; // 获取tid只有在线程活着的时候才可以
    safe_queue<task_t<parameter_name, result_name>*>* sq; // 线程的任务队列指针
    pthread_spinlock_t status_lock; // 锁线程状态
};
#endif