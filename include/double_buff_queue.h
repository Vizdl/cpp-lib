#ifndef _DOUBLE_BUFF_QUEUE_
#define _DOUBLE_BUFF_QUEUE_
#include <queue>
#include <pthread.h>
using namespace std;
/**
 * 多读多写双缓存队列
 * 两个队列,两个锁,一个读锁,一个写锁。读锁保护read_que,写锁保护write_que。
 * 当读时发现 read_que.empty(),发生队列交换。当发生交换时,禁止读也禁止写,需要同时持有读锁和写锁。
 */
template<typename element_type>
class double_buff_queue {
private : 
    queue<element_type>* read_que;  // 读队列,所有读操作依赖这个指针
    queue<element_type>* write_que; // 写队列,所有写操作依赖这个指针
    pthread_mutex_t read_mutex;     // 保护 read_que
    pthread_mutex_t write_mutex;    // 保护 write_que
    pthread_cond_t cond;            // 两个队列为空的时候锁上
public :
    double_buff_queue(){
        read_que = new queue<element_type>();
        write_que = new queue<element_type>();
    }
    ~double_buff_queue(){
        delete read_que;
        delete write_que;
    }
    element_type pop();
    void push(element_type element);
};

/**
 * 如若两个链表都为空,则堵塞。
 * 堵塞在哪?
 * 堵塞在写锁,且不释放读锁，其他读的线程会被mutex切换到堵塞态睡眠,而堵塞在条件变量的会等待唤醒然后抢占任务最后释放读锁。
 * 如若一个链表为空,则交换队列。
 * 如若不为空则读取
 */
template<typename element_type>
element_type double_buff_queue<element_type>::pop(){
    pthread_mutex_lock(&read_mutex);
    if (read_que->empty()){
        // 加写锁看看写队列是否为空
        pthread_mutex_lock(&write_mutex);
        while (write_que->empty()){
            pthread_cond_wait(&cond, &write_mutex);
        }
        // 写锁不为空，交换读写队列
        queue<element_type>* temp = read_que;
        read_que = write_que;
        write_que = temp;
        // 释放写锁
        pthread_mutex_unlock(&write_mutex);
    }
    // 此时读队列不为空
    element_type res = read_que->front();
    read_que->pop();
    pthread_mutex_unlock(&read_mutex);
    return res;
}

template<typename element_type>
void double_buff_queue<element_type>::push(element_type element){
    pthread_mutex_lock(&write_mutex);
    write_que->push(element);
    pthread_mutex_unlock(&write_mutex);
    pthread_cond_signal(&cond);
}

#endif
