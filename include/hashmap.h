#include "list.h"
#include "atomic.h"
#include <pthread.h>
#include <memory.h>

#define ATOMIC 1

// 2 的次方
#define BUCKET_SIZE 131072
#define HASH(key) (key & (BUCKET_SIZE - 1))

int counter = 0;

struct node {
    int key;                // 该数据不可更改
    int val;  
    int refcnt;             // 引用计数,这里的引用计数真的能保护释放么?并不能,如若 node 已经被释放了,那么访问 node->refcnt 本身就是一种错误！！！
    pthread_mutex_t lock;
    struct list_head list;  // 该数据受到桶内的锁保护
};

struct bucket {
    struct list_head head;
    pthread_mutex_t lock;
};

struct hashmap {
    struct bucket buckets[BUCKET_SIZE];
};

#define mb() 	asm volatile("mfence":::"memory")
#define rmb()	asm volatile("lfence":::"memory")
#define wmb()	asm volatile("sfence":::"memory")

struct node* create_node (int key, int val) {
    struct node *node = (struct node *)malloc(sizeof(struct node));
    node->key = key;
    node->val = val;
    node->refcnt = 1;
    pthread_mutex_init(&node->lock, NULL);
    return node;
}

void node_release(struct node *p) {
#ifdef ATOMIC
    int new_count = add(&p->refcnt, -1);
    if (new_count == 1) {
        free(p);
        add(&counter,1);
    }
#else
    int new_count = __sync_sub_and_fetch(&p->refcnt, 1);
    if (new_count == 0) {
        free(p);
        __sync_add_and_fetch(&counter,1);
    }
#endif
}

void node_add_ref(struct node *p) {
#ifdef ATOMIC
    add(&p->refcnt, 1);
    // 这里是否需要加写内存屏障来保证refcnt真正地写到内存里去了,防止在 node_release 时会读出更小的 refcnt 值?
    // 这里不需要加内存屏障,因为在 x86 中使用 lock 指令时会自动加一个完整的内存屏障(MFENCE)。
    // wmb();
#else
    __sync_add_and_fetch(&p->refcnt, 1);
#endif
}

static struct node* find_by_key(struct hashmap* hm, int key) {
    int idx = HASH(key);
    struct list_head* pos;
    struct node* p = NULL;
    struct bucket* bucket = &hm->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    list_for_each(pos, &bucket->head) {
        p = container_of(pos, struct node, list);
        if (p->key == key) {
            node_add_ref(p);
            break;
        }
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

int hashmap_insert(struct hashmap *hm, struct node *p) {
    int idx = HASH(p->key);
    struct bucket* bucket = &hm->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    node_add_ref(p);
    list_add_tail(&p->list, &bucket->head);
    pthread_mutex_unlock(&bucket->lock);
}

int hashmap_inc(struct hashmap* hm, int key) {
    struct node *p = find_by_key(hm, key);
    if (!p) {
        return 0;
    }
    pthread_mutex_lock(&p->lock);
    p->val++;
    pthread_mutex_unlock(&p->lock);
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
        if (p->key == key) {
            node_add_ref(p);
            break;
        }
        p = NULL;
    }
    if (p != NULL) {
        list_del(&p->list);
        node_release(p);
    }
    pthread_mutex_unlock(&bucket->lock);
    return p;
}