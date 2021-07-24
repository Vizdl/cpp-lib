#ifndef _THREAD_POOL_TASK_
#define _THREAD_POOL_TASK_
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

#endif