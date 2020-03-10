#include<iostream>
#include"thread_pool.h"
using namespace std;

#ifdef _SECOND_
#define MAX  10000
// 第一个元素为当前数组大小
int func(int a){
    return a + 1;
}


void* func (void* ptr){

}

void test2(){
    // 创建任务
    task_t<int, int>* tt[MAX];
    // 创建线程池
    thread_pool<int, int>* pool = new thread_pool<int, int>();
    for (int i = 0; i < MAX; i++){
        tt[i] = new task_t<int, int>(func, i);
        pool->add_task(tt[i]);
    }
    for (int i = 0; i < MAX; i++){
        int a = tt[i]->get_result();
        cout << a << endl;
        delete tt[i];
    }
    // sleep(100);
    cout<< "end" << endl;
    delete pool;
}
#endif

#ifdef _FIRST_
#define MAX  1000
//第一个元素为当前数组大小
int func(int* a){
    int len = a[0];
    int res = 0;
    for(int i = 1; i < len; i++)
        res += a[i];
    return res;
}
void test1(){
    // 创建任务
    task_t<int*, int>* tt[MAX];
    int argv[MAX][MAX]; // MAX个任务的参数
    // 创建线程池
    thread_pool<int*, int>* pool = new thread_pool<int*, int>();
    for (int i = 0; i < MAX; i++){
        // 初始化当前参数
        argv[i][0] = MAX;
        for (int j = i + 1; j < i + MAX; j++){
            argv[i][j - i] = j;
#ifdef _DEBUG_
            cout << (j - i) << " ";
#endif
        }
#ifdef _DEBUG_
        cout << endl;
#endif
        tt[i] = new task_t<int*, int>(func, argv[i]);
        pool->add_task(tt[i]);
    }
    for (int i = 0; i < MAX; i++){
        int a = tt[i]->get_result();
        cout << a << endl;
        delete tt[i];
    }
    cout<< "end" << endl;
    delete pool;
}

#endif
int main(){
#ifdef _FIRST_
    test1();
#endif
#ifdef _SECOND_
    test2();
#endif
    return 0;
}
