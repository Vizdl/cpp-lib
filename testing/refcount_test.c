#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 100

typedef struct {
    int ref_count;
    int value;
    pthread_mutex_t lock;
} shared_data_t;

shared_data_t* shared_data_create(int value) {
    shared_data_t* shared_data = (shared_data_t*) malloc(sizeof(shared_data_t));
    shared_data->ref_count = 1;
    shared_data->value = value;
    pthread_mutex_init(&shared_data->lock, NULL);
    return shared_data;
}

void shared_data_add_ref(shared_data_t* shared_data) {
    __sync_add_and_fetch(&shared_data->ref_count, 1);
}

void shared_data_release(shared_data_t* shared_data) {
    int new_count = __sync_sub_and_fetch(&shared_data->ref_count, 1);
    if (new_count == 0) {
        pthread_mutex_destroy(&shared_data->lock);
        free(shared_data);
    }
}

void shared_data_set_value(shared_data_t* shared_data, int value) {
    pthread_mutex_lock(&shared_data->lock);
    shared_data->value = value;
    pthread_mutex_unlock(&shared_data->lock);
}

int shared_data_get_value(shared_data_t* shared_data) {
    pthread_mutex_lock(&shared_data->lock);
    int value = shared_data->value;
    pthread_mutex_unlock(&shared_data->lock);
    return value;
}

void* thread_func(void* arg) {
    shared_data_t* shared_data = (shared_data_t*) arg;
    shared_data_add_ref(shared_data);
    printf("Thread %ld: value = %d, ref_count = %d\n", pthread_self(), shared_data_get_value(shared_data), shared_data->ref_count);
    shared_data_release(shared_data);
    return NULL;
}

int main() {
    shared_data_t* shared_data = shared_data_create(0);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, shared_data);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    shared_data_release(shared_data);
    return 0;
}