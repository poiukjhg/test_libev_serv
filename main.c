#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>     
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>         
#include <sys/socket.h>
#include <string.h>
#include "mytype.h"
#include "mylogs.h"
#include "mylock.h"
#define USEING_EV
#ifdef USEING_EV
#include "my_ev_handler.h"
#else
#include "my_event_handler.h"
#endif
#define PROCESS_NUM 2

int init_listen(int port)
{
	int listen_fd = -1;
	struct sockaddr_in sin; 
	int flags;
	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);	
	if ((flags = fcntl(listen_fd, F_GETFL, NULL)) < 0) {
		handle_error("fcntl get");
		exit(-1);
	}
	if (!(flags & O_NONBLOCK)) {
		if (fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			handle_error("fcntl set");
			exit(-1);
		}
	}                      
	if(bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin))<0){
		handle_error("bind port");
		exit(-1);
	}
	if( listen(listen_fd, 16) == -1){  
		handle_error("listen");
		exit(-1);
	} 	
	return listen_fd;
}	

int test_read_event(char *recv_str, size_t recv_len, read_userdata *read_data)
{
	int index = 0;
	char *response_str = "HTTP/1.1 200 ok\n\r\n\r";
	/*
	for(index = 0; index < recv_len; index++){
		printf("%c", recv_str[index]);
	}
	*/
	//printf("receive len = %d pid = %d accept = %d\n\r", (int)recv_len, (int)getpid(), read_data->my_bs->accept_fd_num);
	read_data->response = response_str;
	read_data->send_len = strlen(response_str);
	read_data->close_fd = 1;
	return SEND_RESPONSE;
}

int main()
{
	int process_num = PROCESS_NUM;
	int process_id[PROCESS_NUM] = {0};
	int index = 0;
	int tmp_fd = 0;
	void *lock = NULL;
	my_lock_init(&lock);
	int listen_fd = -1;	
	int stat;
	pid_t pid;
	listen_fd = init_listen(8000);
	printf("bind 8000\n\r");		
	for (index = 0; index < process_num; index ++){
		tmp_fd = fork();
		if (tmp_fd == 0){
			
			my_base *bs = NULL;				
			bs = server_init();	
			bs->lock = lock;
			if (bs == NULL){
				printf("base c is NULL\n\r");
				exit(-1);
			}
			server_listen_fd_add(bs, listen_fd);
			server_rfunc_add(bs, listen_fd, test_read_event);
			server_loop_cb_set(bs);
			server_start(bs);
			printf("pid %d end\n\r", (int)getpid());
		}
		else if (tmp_fd >0){
			process_id[index] = tmp_fd;
			printf("child pid = %d\n\r", process_id[index]);
			continue;
		}

	}
	printf("out fork %d\n\r", (int)getpid());
	while((pid = wait(&stat)) > 0){ 
		printf("child %d terminated\n\r", pid); 
		if (WIFEXITED(stat))
			printf("child exited normal exit status=%d\n", WEXITSTATUS(stat));

		else if (WIFSIGNALED(stat))
			printf("child exited abnormal signal number=%d\n", WTERMSIG(stat));
		else if (WIFSTOPPED(stat))
			printf("child stoped signal number=%d\n", WSTOPSIG(stat));			   
       }
	return 0;
}

