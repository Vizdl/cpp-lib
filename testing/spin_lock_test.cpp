# include "spin_lock.h"
using namespace std;
int main(){
    unsigned int spinlock = LOCK_INIT;
    lock(&spinlock);
    lock(&spinlock);
    unlock(&spinlock);
    unlock(&spinlock);
    return 0;
}
