#include "rw_lock.h" 
#include<iostream>

using namespace std;

rw_lock* rw; // 全局读写锁

void* call_back1 (void* arg){
    rw->r_lock();
    usleep(10);
    rw->r_unlock();
    return 0;
}
void* call_back2 (void* arg){
    rw->w_lock();
    usleep(10);
    rw->w_unlock();
    return 0;
}

#define R_COUNT 1000
#define W_COUNT 3000
int main (){
    rw = new rw_lock();
    pthread_t tid1[R_COUNT], tid2[W_COUNT];
    for (int i = 0; i < W_COUNT; i++){
        pthread_create(&tid2[i], NULL, call_back2, NULL);
    }
    for (int i = 0; i < R_COUNT; i++){
        pthread_create(&tid1[i], NULL, call_back1, NULL);
    }

    sleep(30); // 等待
    delete rw;
    return 0;
}