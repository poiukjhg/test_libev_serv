#ifndef MY_ERROR_H
#define MY_ERROR_H
#include <errno.h>
#include<stdlib.h>
#include<stdio.h>


#define handle_error(msg)  \
	do { \
		perror(msg); \
		exit(EXIT_FAILURE); \
	} while (0)
#define log_output(msg)  \
	do { \
		printf("%s\n\r", msg); \
	} while (0)	
#endif		   	
