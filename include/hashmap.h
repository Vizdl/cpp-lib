#include "list.h"
#include <pthread.h>
#include <memory.h>
#include "spin_lock.h"
#include "atomic.h"
// 2 的次方
#define BUCKET_SIZE 8192
#define HASH(key) (key & (BUCKET_SIZE - 1))

int counter = 0;

struct node {
    int key;                // 该数据不可更改
    int val;                
    int refcnt;             // 引用计数
    struct spinlock lock;   // 该锁保护 val
    struct list_head list;  // 该数据受到桶内的锁保护
};

struct bucket {
    struct list_head head;
    pthread_mutex_t lock;
};

struct hashmap {
    struct bucket buckets[BUCKET_SIZE];
};

struct node* create_node (int key, int val) {
    struct node *node = (struct node *)malloc(sizeof(struct node));
    node->key = key;
    node->val = val;
    node->refcnt = 1;
    init_lock(&node->lock);
    return node;
}

void node_release(struct node *p) {
    int new_count = __sync_sub_and_fetch(&p->refcnt, 1);
    if (new_count == 0) {
        free(p);
        add(&counter,1);
    }
}

void node_add_ref(struct node *p) {
    __sync_add_and_fetch(&p->refcnt, 1);
}

static struct node* find_by_key(struct hashmap* hm, int key) {
    int idx = HASH(key);
    struct list_head* pos;
    struct node* p = NULL;
    struct bucket* bucket = &hm->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    list_for_each(pos, &bucket->head) {
        p = container_of(pos, struct node, list);
        if (p->key == key)
            break;
        p = NULL;
    }
    pthread_mutex_unlock(&bucket->lock);
    return p;
}

static int init_bucket(struct bucket* bucket) {
    INIT_LIST_HEAD(&bucket->head);
    pthread_mutex_init(&bucket->lock, NULL);
    return 0;
}

int init_hashmap(struct hashmap* hm) {
    int i;
    for (i = 0; i < BUCKET_SIZE; i++) {
        init_bucket(&hm->buckets[i]);
    }
    return 0;
}

int hashmap_insert(struct hashmap *hm, struct node *node) {
    int idx = HASH(node->key);
    struct bucket* bucket = &hm->buckets[idx];
    node_add_ref(node);
    pthread_mutex_lock(&bucket->lock);
    list_add_tail(&node->list, &bucket->head);
    pthread_mutex_unlock(&bucket->lock);
}

int hashmap_inc(struct hashmap* hm, int key) {
    struct node *p = find_by_key(hm, key);
    if (!p) {
        // printf("hashmap inc find null : %d\n", key);
        return 0;
    }
    node_add_ref(p);
    spin_lock(&p->lock);
    p->val++;
    spin_unlock(&p->lock);
    node_release(p);
}

struct node* hashmap_remove(struct hashmap* hm, int key) {
    int idx = HASH(key);
    struct list_head* pos;
    struct node* p = NULL;
    struct bucket* bucket = &hm->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    list_for_each(pos, &bucket->head) {
        p = container_of(pos, struct node, list);
        if (p->key == key)
            break;
        p = NULL;
    }
    if (p != NULL) {
        list_del(&p->list);
    }
    pthread_mutex_unlock(&bucket->lock);
    return p;
}