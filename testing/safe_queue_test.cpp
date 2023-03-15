#include "safe_queue.h"
#include <pthread.h>
#include <iostream>
using namespace std;

safe_queue<int> que;

void* read(void* data){
    int max = *((int*)data);
    int size = 0;
    while (size < max)
    {
        int num = que.pop();
        if (num != size)
            cout << "error : " << num << "," << size << endl;
        size++;
    }
    cout << "yes!!!" << endl;
    return NULL;
}

void* write(void* data){
    int max = *((int*)data);
    for (int i = 0; i < max; i++) 
        que.push(i);
    return NULL;
}

/**
 * 单读单写验证，一个线程读一个线程写。
 * 单读多写验证，一个线程
 * 多读多写验证,开启DEBUG返回一个队列。
 * 
 */
int main (){
    pthread_t read_tid, write_tid;
    void *status;
    int max = 10000000;
    pthread_create(&read_tid, NULL, read, (void*)&max);
    pthread_create(&write_tid, NULL, write, (void*)&max);
    pthread_join(write_tid, &status);
    pthread_join(read_tid, &status);
    return 0;
}