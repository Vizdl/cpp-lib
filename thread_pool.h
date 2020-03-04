#ifndef _THREADPOOL_
#define _THREADPOOL_

#include<iostream>
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
class safe_queue;
/*
task_t 有两个方向操作,
第一 : 线程组调用call_back_func后设置res
第二 : main线程取res
*/
template<typename parameter_name, typename result_name>
class task_t{
public:
    typedef result_name(*call_back_ptr)(parameter_name); // 声明函数指针
    // 构造与析构函数
    task_t(call_back_ptr call_back, parameter_name arg) 
    : call_back_func(call_back), arg(arg), has_res(false){
        // 初始化锁
        pthread_mutex_init(&mut, NULL);
        pthread_cond_init(&data_cond, NULL);
    }
    ~task_t(){}

    // 功能函数
    void set_result(result_name res){
        pthread_mutex_lock(&mut);
        // 如若答案未被设置则堵塞
        if (has_res){
            perror("重复设置答案");
            return ;
        }
#ifdef _DEBUG_
        cout << "set res to " << res << endl;
#endif
        this->res = res;
        // 修改has_res标志
        has_res = true;

        pthread_mutex_unlock(&mut);
        // 通知所有等待的线程
        pthread_cond_broadcast(&data_cond);
    }

    result_name get_result(){
        pthread_mutex_lock(&mut);
        // 如若答案未被设置则堵塞
        while (!has_res){
            pthread_cond_wait(&data_cond, &mut);
        }
#ifdef _DEBUG_
        cout << "get res is " << res << endl;
#endif
        pthread_mutex_unlock(&mut);
        return res;
    }

    call_back_ptr get_call_back_ptr(){
        return this->call_back_func;
    }

    parameter_name get_arg (){
        return this->arg;
    }
private:
    call_back_ptr call_back_func; // 回调函数
    parameter_name arg; // 参数
	pthread_mutex_t mut;      // task_t的锁
    pthread_cond_t data_cond; // 实现异步获取返回值
    result_name res;    // 返回值
    bool has_res;          // 是否已经得到结果
};

/*
线程节点。
*/
template<typename parameter_name, typename result_name>
class thread{
public:
    typedef result_name(*call_back_ptr)(parameter_name); // 声明函数指针
    atomic<bool> is_alive; // 线程是否活着
    safe_queue<task_t<parameter_name, result_name>*>* sq; // 线程的任务队列指针
    // 构造和析构
    thread(safe_queue<task_t<parameter_name, result_name>*>* sq)
    : is_alive(true), sq(sq){}

    void start(){
        // 创建线程
        pthread_t tid;
        // 因为该函数是静态方法,所以里面的所有参数都要是在调用时,保证存在。
        pthread_create(&tid, NULL, thread_call_back, this);
    }
    
private:
    // 该函数的参数为 一个thread对象,这样就能实现一个静态方法却能访问不同的对象。
    static void* thread_call_back(void* arg){
        thread<parameter_name, result_name>* tptr = (thread<parameter_name, result_name>*)arg;
        int i = 0;
        while (tptr->is_alive){
            task_t<parameter_name, result_name>* task = tptr->sq->pop();
            // 要保证 task 存在
            if (task == NULL){
                // printf("第几次进入 %d task is NULL\n", i);
                perror("task is NULL");
            }
            result_name res = task->get_call_back_ptr()(task->get_arg()); // 调用任务
            task->set_result(res); // 设置结果。
            i++;
        }
        return NULL;
    }
};


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
            pthread_cond_wait(&queue_is_empty, &mutex);
        }
        // cout << "dwqjkdpo" << endl;
        element_type res = que->front();
        que->pop();
        pthread_mutex_unlock(&mutex);
        return res;
    }

    void push(element_type element){
        pthread_mutex_lock(&mutex);
        que->push(element);
        pthread_mutex_unlock(&mutex);
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


//
template<typename parameter_name, typename result_name>
class thread_pool{
public:
#define THREAD_INIT_SIZE sysconf( _SC_NPROCESSORS_CONF)
    // 构造和析构函数
    thread_pool()
    : task_queue(new safe_queue<task_t<parameter_name, result_name>*>()),
     thread_queue(new safe_queue<thread<parameter_name, result_name>*>()){
         // 往线程队列添加线程
         for (int i = 0; i < THREAD_INIT_SIZE; i++){
            thread<parameter_name, result_name>* t = new thread<parameter_name, result_name>(task_queue);
            t->start();
            thread_queue->push(t); 
         }
    }
    ~thread_pool(){
        delete task_queue;
        delete thread_queue;
    }

    // 功能函数
    void add_task(task_t<parameter_name, result_name>* task){
        task_queue->push(task);
    }

private:
    safe_queue<task_t<parameter_name, result_name>*>* task_queue; // 线程安全队列
    safe_queue<thread<parameter_name, result_name>*>* thread_queue;
    atomic<bool> is_alive; // 线程池是否活着
};

#endif
