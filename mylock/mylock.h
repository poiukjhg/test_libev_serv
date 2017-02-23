#ifndef MYLOCK_H
#define MYLOCK_H
#include <sys/mman.h>
#include <pthread.h>  
pthread_mutex_t* mymutex;//
pthread_mutexattr_t mutexattr;//

int my_lock_init();
int my_lock_trylock();
int my_lock_tryunlock();
void my_lock_destroy();
#endif