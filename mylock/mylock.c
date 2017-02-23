#include <stdlib.h>
#include <sys/stat.h>     
#include <unistd.h>       
#include "../mytype.h"
#include "../myerror.h"
#include "mylock.h"



int my_lock_init()
{
	pthread_mutexattr_init(&mutexattr);//init mutex  attr   
   	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);//set process_shared   	
	mymutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, 
					MAP_SHARED|MAP_ANONYMOUS , -1, 0);
	if (mymutex == NULL){
		handle_error("mmap");
		return RES_ERROR;
	}
	pthread_mutex_init(mymutex, &mutexattr);
	return RES_OK;
}

int my_lock_trylock()
{
	int res = RES_OK;
	if (mymutex != NULL){		
		if((res = pthread_mutex_trylock(mymutex) ) != RES_OK)
			return RES_ERROR;
		else return RES_OK;
	}
	else return RES_ERROR;		
}
int my_lock_tryunlock()
{
	int res = RES_OK;
	if (mymutex != NULL){		
		if((res = pthread_mutex_unlock(mymutex) ) != RES_OK)
			return RES_ERROR;
		else return RES_OK;
	}
	else return RES_ERROR;
}

void my_lock_destroy()
{
	munmap(mymutex, sizeof(pthread_mutex_t)); 
}


