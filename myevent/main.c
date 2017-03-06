#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>     
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <string.h>
#include "../mytype.h"
#include "../mylogs.h"
#include "my_event_handler.h"
#include "mylock.h"
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
	char *response_str = "200, ok\n\r";
	for(index = 0; index < recv_len; index++){
		printf("%c", recv_str[index]);
	}
	printf("\n\rreceive len = %d\n\r", (int)recv_len);
	read_data->response = response_str;
	read_data->send_len = strlen(response_str);
	read_data->close_fd = 1;
	return SEND_RESPONSE;
}

int main()
{
	int listen_fd = -1;	
	my_base *bs = NULL;
	void *lock = NULL;
	my_lock_init(&lock);
	bs = server_init();	
	bs->lock = lock;
	if (bs == NULL){
		log_output("base c is NULL");
		exit(-1);
	}
	listen_fd = init_listen(8000);
	printf("bind 8000\n\r");	
	server_listen_fd_add(bs, listen_fd);
	server_rfunc_add(bs, listen_fd, test_read_event);
	
	listen_fd = init_listen(9000);
	printf("bind 9000\n\r");
	server_listen_fd_add(bs, listen_fd);
	server_rfunc_add(bs, listen_fd, test_read_event);	
	server_loop_cb_set(bs);	
	server_start(bs);	
	return 0;	
}

