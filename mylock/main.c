#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>     
#include <unistd.h>    
#include <sys/mman.h>
#include <pthread.h>  
#include "../mytype.h"
#include "../myerror.h"
#include "mylock.h"

int  main()
{
	int fd = 0;
	int res = RES_ERROR;	
	int *test_param = (int *)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
					MAP_SHARED|MAP_ANONYMOUS , -1, 0);	
	my_lock_init();
	fd = fork();
	if (fd <0){
		printf("fork error\n\r");
		exit(-1);
	}
	else if (fd == 0){
		while((res =my_lock_trylock()) != RES_OK){
			printf("lock fail\n\r");
			sleep(1);
		}
		*test_param = 2;
		printf("resr = %d\n\r", *test_param);
		my_lock_tryunlock();
		sleep(3);
		return 0;
	}
	else{
		while((res =my_lock_trylock()) != RES_OK){
			printf("lock fail\n\r");
			sleep(1);
		}
		*test_param = 1;
		printf("child resr = %d\n\r", *test_param);	
		sleep(3);
		my_lock_tryunlock();
		return 0;
	}
}


