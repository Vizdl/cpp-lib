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
            pthread_mutex_unlock(&mut);
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


// 
template<typename parameter_name, typename result_name>
class thread_pool{
public:
#define THREAD_INIT_SIZE sysconf( _SC_NPROCESSORS_CONF)
    // 构造和析构函数
    thread_pool()
    : task_queue(new safe_queue<task_t<parameter_name, result_name>*>()),
     thread_queue(new safe_queue<thread<parameter_name, result_name>*>()),
     is_alive(true){
         // 初始化线程池的锁
        pthread_mutex_init(&mutex, NULL);

        // 往线程队列添加线程
        for (int i = 0; i < THREAD_INIT_SIZE; i++){
            thread<parameter_name, result_name>* t = new thread<parameter_name, result_name>(task_queue);
            thread_queue->push(t); 
        }
    }
    ~thread_pool(){
        delete task_queue;
        delete thread_queue;
    }

    // 功能函数
    void add_task(task_t<parameter_name, result_name>* task){
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
            thread<parameter_name, result_name>* now_pthread;
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
    safe_queue<task_t<parameter_name, result_name>*>* task_queue; // 安全  线程队列
    safe_queue<thread<parameter_name, result_name>*>* thread_queue; // 
    bool is_alive; // 线程池是否活着
    pthread_mutex_t mutex; // 主要用来保护is_alive状态的。
}; 

#endif
