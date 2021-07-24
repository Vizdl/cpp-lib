#ifndef _THREADPOOL_
#define _THREADPOOL_

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
#include "thread_pool_thread.h"

using namespace std;

template<typename parameter_name, typename result_name, 
typename task_type = task_t<parameter_name, result_name>,
typename thread_type = thread<parameter_name, result_name>>
class thread_pool{
public:
#define THREAD_INIT_SIZE sysconf( _SC_NPROCESSORS_CONF) /* CPU核数 */
    // 构造和析构函数
    thread_pool()
    : task_queue(new safe_queue<task_type*>()),
     thread_queue(new safe_queue<thread_type*>()),
     is_alive(true){
         // 初始化线程池的锁
        pthread_mutex_init(&mutex, NULL);

        // 往线程队列添加线程
        for (int i = 0; i < THREAD_INIT_SIZE; i++){
            thread_type* t = new thread_type(task_queue);
            thread_queue->push(t); 
        }
    }
    ~thread_pool(){
        delete task_queue;
        delete thread_queue;
    }

    // 功能函数
    void add_task(task_type* task){
        pthread_mutex_lock(&mutex);
        if (is_alive)
            task_queue->push(task);
        else 
            perror("不允许在线程池死亡状态下添加任务");
        pthread_mutex_unlock(&mutex);
    }
    // 等待任务队列为空后自然死亡,并且在此期间内无法添加任务。
    void wait_to_die(){
        pthread_mutex_lock(&mutex);
        if (is_alive){
            // 把阻塞在任务队列上的线程用信号杀死,其他正在执行任务的线程只需要设置延时死亡就可以了。
            thread_type* now_pthread;
            while (!thread_queue->empty()){
                now_pthread = thread_queue->pop();
                // 获取当前tid

                // 其实这里需要判断线程是否休眠
                if (now_pthread->thread_is_sleep){
                    pthread_kill(now_pthread->tid, SIGKILL);
                }else {
                    now_pthread->wait_to_die();
                }
            }
        }else{
            perror("请勿重复设置线程池死亡。");
        }
        pthread_mutex_unlock(&mutex);
    }

private:
    safe_queue<task_type*>* task_queue; // 安全  线程队列
    safe_queue<thread_type*>* thread_queue; // 
    bool is_alive; // 线程池是否活着
    pthread_mutex_t mutex; // 主要用来保护is_alive状态的。
}; 

#endif
