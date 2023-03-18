#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include "hashmap.h"

struct hashmap hm;

#define THREAD_COUNT 300
pthread_t threadid[THREAD_COUNT] = {0};

void* thread_callback(void *arg){
    int i = 0;
    int data = (int)arg + 1;

    while(i++ < BUCKET_SIZE){
        struct node *node = alloc_node(data * BUCKET_SIZE + i, 1);
        hashmap_insert(&hm, node);
    }
}



int main (){
    int i = 0;
    void *status;
    struct list_head* pos;
    int sum = 0;

    init_hashmap(&hm);

    // 创建线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_callback, (void*)i);
    }
    // 等待线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }

    for (i = 0; i < BUCKET_SIZE; i++) {
        list_for_each(pos, &hm.buckets[i].head) {
            struct node* p = container_of(pos, struct node, list);
            sum += p->val;
        }
    }
    if (sum == THREAD_COUNT * (BUCKET_SIZE))
        printf("yes : sum : %d\n", sum);
    else
        printf("error : sum : %d\n", sum);
    
    return 0;
}