#ifndef MY_ERROR_H
#define MY_ERROR_H
#include <errno.h>
#include<stdlib.h>
#include<stdio.h>

#define DEBUG
#undef DEBUG
#define handle_error(msg)  \
	do { \
		perror(msg); \
		printf("exit\n\r"); \
		exit(EXIT_FAILURE); \
	} while (0)
	
#ifdef DEBUG	

#define log_output(format, ...)  \
	do { \
		printf (format, ##__VA_ARGS__); \
	} while (0)	
#else
#define log_output(msg) \
	do { \
	} while (0)	
#endif

#endif