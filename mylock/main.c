#include <stdlib.h>
#include <sys/stat.h>     
#include <unistd.h>       
#include "../mytype.h"
#include "../myerror.h"
#include "mylock.h"

void main()
{
	int fd = 0;
	int res = RES_ERROR;
	int test_param = 0;
	my_lock_init();
	fd = fork();
	if (fd <0){
	}
	else if (fd == 0){
	}
	else{
	}
}


