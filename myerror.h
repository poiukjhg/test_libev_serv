#ifndef MY_ERROR_H
#define MY_ERROR_H

#include <stdlib.h>
#include <errno.h>

#define handle_error(msg) \
	do { 
		perror(msg); 
		exit(EXIT_FAILURE); 
	} while (0)
#endif		   	
