#include "list.h"
#include "spin_lock.h"
#include <memory.h>

// 2 的次方
#define BUCKET_SIZE 4096
#define HASH(key) (key & (BUCKET_SIZE - 1))

struct node {
    int key;                // 该数据不可更改
    int val;                
    // struct spinlock lock;// 该锁保护 val
    struct list_head list;  // 该数据受到桶内的锁保护
};

struct bucket {
    struct list_head head;
    struct spinlock lock;
};

struct hashmap {
    struct bucket buckets[BUCKET_SIZE];
};

static int init_bucket(struct bucket* bucket) {
    INIT_LIST_HEAD(&bucket->head);
    init_lock(&bucket->lock);
    return 0;
}

struct node* alloc_node (int key, int val) {
    struct node *node = (struct node *)malloc(sizeof(struct node));
    node->key = key;
    node->val = val;
    // init_lock(&node->lock);
    return node;
}

int init_hashmap(struct hashmap* hm) {
    int i;
    for (i = 0; i < BUCKET_SIZE; i++) {
        init_bucket(&hm->buckets[i]);
    }
    return 0;
}

int hashmap_insert(struct hashmap *hm, struct node *node) {
    int key = HASH(node->key);
    struct bucket* bucket = &hm->buckets[key];
    spin_lock(&bucket->lock);
    list_add_tail(&node->list, &bucket->head);
    spin_unlock(&bucket->lock);
}


// int hashmap_remove(struct hashmap* hm, int key) {
//     list_for_each
// }
 
// #define hashmap_for_each(idx, pos, hm) \
//     for (idx = 0, pos = &((hm)->buckets[idx].head); idx < BUCKET_SIZE; \
//     pos->next ? (++idx < BUCKET_SIZE ? (pos = &((hm)->buckets[idx].head)) : (pos = NULL)) : (pos = pos->next))
