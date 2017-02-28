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
#include "my_ev_handle.h"

int test_read(char *str)
{
	printf("receive %s\n\r", str);
	return strlen(str);
}
int test_get_staus(char* str){
	return 1;
}
	
int main(){
	int listen_fd = -1;
	struct sockaddr_in sin; 
	int flags;
	my_event_base *my_ev_base;
	buffer_cb_func *my_cb_func;
	r_buffer_cb_func* my_r_buffer_cb_func;
	
	my_ev_base = my_base_init();
	if (my_ev_base == NULL){
		exit(-1);
	}
	my_cb_func = &(my_ev_base->cb_func);
	my_r_buffer_cb_func = my_cb_func->r_func;
	my_r_handler_helper_addtail(&my_r_buffer_cb_func, test_read);	
	my_get_cur_status_helper_set(&my_cb_func, test_get_staus);	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(8000);
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
		handle_error("bind");
		exit(-1);
	}
	if( listen(listen_fd, 16) == -1){  
		handle_error("listen");
		exit(-1);
	} 
	my_start_listen(listen_fd, my_ev_base);
	return 0;
}

