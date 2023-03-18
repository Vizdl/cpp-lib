#ifndef __SPINLOCK__
#define __SPINLOCK__

#include <stdint.h>
#include <stdio.h>
// 带参宏 
// <==> cmpxchg(void* ptr, int old, int new)
#define cmpxchg( ptr, _old, _new ) ({                       \
  volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);    \
  uint32_t __ret;                                           \
  asm volatile( "lock; cmpxchgl %2,%1"                      \
    : "=a" (__ret), "+m" (*__ptr)                           \
    : "r" (_new), "0" (_old)                                \
    : "memory"  				                            \
  );                                                        \
  __ret;                                                    \
})

#define setZero( ptr )                                      \
    volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);  \
    asm volatile(                                           \
    "movl %1,%0":                                           \
    : "m"(*__ptr), "I"(0)                                   \
    : "memory"                                              \
)

#define LOCK_INIT 0

struct spinlock {
    unsigned int flags;
};

void init_lock(struct spinlock* lock){
    lock->flags = 0;
}

void spin_lock(struct spinlock* lock){
    while (cmpxchg(&lock->flags, 0, 1))
    {
#ifdef __DEBUG__
        printf("sleep on\n");
#endif
    }
#ifdef __DEBUG__
    printf("lock\n");
#endif
}

void spin_unlock(struct spinlock* lock){
    setZero(&lock->flags);
}

#endif