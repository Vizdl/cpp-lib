#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include "hashmap.h"

struct hashmap hm;

#define THREAD_COUNT 30
pthread_t threadid[THREAD_COUNT] = {0};

void* thread_insert(void *arg){
    int i = 0;
    int data = (int)arg;
    for (i = 0; i < BUCKET_SIZE; i++){
        struct node *node = alloc_hm_node(data * BUCKET_SIZE + i, 1);
        hashmap_insert(&hm, node);
        put_hm_node(node);
    }
}

void* thread_inc(void *arg){
    int i, j;
    for (i = 0; i < THREAD_COUNT; i++)
        for (j = 0; j < BUCKET_SIZE; j++)
            hashmap_inc(&hm, i * BUCKET_SIZE + j);
}

void* thread_remove(void *arg){
    int i, j;
    for (i = 0; i < THREAD_COUNT; i++)
        for (j = 0; j < BUCKET_SIZE; j++) {
            struct node *node = hashmap_remove(&hm, i * BUCKET_SIZE + j);
            put_hm_node(node);
        }
}

int main (){
    int i = 0;
    void *status;
    struct list_head* pos;
    int sum = 0;
    pthread_t pid;

    init_hashmap(&hm);

    // 多写
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_insert, (void*)i);
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
#if 0
    // 多修改
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_inc, NULL);
    }
    // 等待线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }
    sum = 0;
    for (i = 0; i < BUCKET_SIZE; i++) {
        list_for_each(pos, &hm.buckets[i].head) {
            struct node* p = container_of(pos, struct node, list);
            sum += p->val;
        }
    }
    if (sum == (THREAD_COUNT + 1) * THREAD_COUNT * (BUCKET_SIZE))
        printf("yes : sum : %d\n", sum);
    else
        printf("error : sum : %d\n", sum);
#endif

#if 0
    // 多删
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_remove, (void*)i);
    }
    // 等待线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }
    sum = 0;
    for (i = 0; i < BUCKET_SIZE; i++) {
        list_for_each(pos, &hm.buckets[i].head) {
            struct node* p = container_of(pos, struct node, list);
            sum += p->val;
        }
    }
    if (sum == 0)
        printf("yes : sum : %d\n", sum);
    else
        printf("error : sum : %d\n", sum);
#else
    // 多删
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_inc, NULL);
    }
    pthread_create(&pid, NULL, thread_remove, (void*)i);
    // 等待线程
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_join(threadid[i], &status);
    }
    pthread_join(pid, &status);
    sum = 0;
    for (i = 0; i < BUCKET_SIZE; i++) {
        list_for_each(pos, &hm.buckets[i].head) {
            struct node* p = container_of(pos, struct node, list);
            sum += p->val;
        }
    }
    if (counter != THREAD_COUNT * (BUCKET_SIZE))
        printf("error : free = %d\n", counter);
    else
        printf("yes : free = %d\n", counter);

    // 能到达这里说明没问题
    if (sum == 0)
        printf("yes : sum : %d\n", sum);
    else
        printf("error : sum : %d\n", sum);
#endif
    return 0;
}