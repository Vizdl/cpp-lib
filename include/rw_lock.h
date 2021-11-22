#ifndef _RW_LOCK_
#define _RW_LOCK_

#include<iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

using namespace std;

/*
如何实现写优先 ? 
    写只要当前不是写锁占据,就先把writecount++,然后等待写锁释放.
    而有写的话,那么其他读锁就加不上去。

读锁条件 : 当前没有写锁, writecount == 0
写锁条件 : 当前没有写锁, 先把坑给占了。然后等待读锁释放。
*/

class rw_lock{
private:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int r_count, w_count;
public:
    rw_lock() : r_count(0), w_count(0){
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&cond, NULL);
    }
    void r_lock(){
        pthread_mutex_lock(&mutex);
        // 看看写锁是否被占据(预备写锁也不能被占据)
        while (w_count > 0){
            pthread_cond_wait(&cond, &mutex); // 如若没有则等待
        }
        r_count++;
#ifdef __DEBUG__
        cout << "read lock is lock!" << endl;
#endif
        pthread_mutex_unlock(&mutex);
    }

    void w_lock(){
        // 看看是否有写
        pthread_mutex_lock(&mutex);
        while (w_count > 0){
            pthread_cond_wait(&cond, &mutex); // 如若没有则等待
        }
        // 当前如若没有写锁占据,则占据写锁。
        w_count++;
        // 看看是否有读锁
        while (r_count > 0){
            pthread_cond_wait(&cond, &mutex); // 如若没有则等待
        }
        // 没有读锁也没有写锁
#ifdef __DEBUG__
        cout << "write lock is lock!" << endl;
#endif
        pthread_mutex_unlock(&mutex);
    }

    void r_unlock(){
        pthread_mutex_lock(&mutex);
        r_count--;
#ifdef __DEBUG__
        cout << "read lock is unlock!" << endl;
#endif
        pthread_cond_broadcast(&cond); // 唤醒队列上所有的元素
        pthread_mutex_unlock(&mutex);
    }

    void w_unlock(){
        pthread_mutex_lock(&mutex);
        w_count--;
#ifdef __DEBUG__
        cout << "write lock is unlock!" << endl;
#endif
        pthread_cond_broadcast(&cond); // 唤醒队列上所有的元素
        pthread_mutex_unlock(&mutex);
    }
};

#endif