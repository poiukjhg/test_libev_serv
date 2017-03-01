#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>     
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <string.h>
//#include <event2/event.h>
#include "../mytype.h"
#include "../mylogs.h"
#include "my_ev_handle.h"

int test_read(char *str, size_t recv_len, send_reply_helper send_func, read_userdata *read_data)
{
	int index = 0;
	char *response_str = "200, ok\n\r";
	for(index = 0; index < recv_len; index++){
		printf("%c", str[index]);
	}
	printf("\n\rreceive len = %d\n\r", (int)recv_len);
	read_data->response = response_str;
	read_data->len = strlen(response_str);
	read_data->if_close_fd = 1;
	send_func(read_data);	
	return recv_len;
}
int test_get_staus(char* str)
{
	return 0;
}

int init_server()
{
	int listen_fd = -1;
	struct sockaddr_in sin; 
	int flags;
	
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
	return listen_fd;
}	

void listenfd_ctrl(int fd, short event, void *arg)
{
	my_base_userdata *my_bs_udata = (my_base_userdata *)arg;
	my_event_base *my_ev_base  =  my_bs_udata->my_ev_base;
	int listen_fd = my_bs_udata->fd;
	my_start_listen(listen_fd, my_ev_base);
}

int main()
{
	int listen_fd = -1;	
	my_event_base *my_ev_base;
	buffer_cb_func *my_cb_func;
	my_base_userdata my_bs_udata;
	//r_buffer_cb_func* my_r_buffer_cb_func;
	
	my_ev_base = my_create_base();
	if (my_ev_base == NULL){
		exit(-1);
	}
	my_cb_func = &(my_ev_base->cb_func);
	//my_r_buffer_cb_func = my_cb_func->r_func;
	my_r_handler_helper_addtail(&(my_cb_func->r_func), test_read);	
	my_get_cur_status_helper_set(&my_cb_func, test_get_staus);	
	listen_fd = init_server();
	my_bs_udata.my_ev_base = my_ev_base;
	my_bs_udata.fd = listen_fd;
	my_base_init_loop_func(my_ev_base, listenfd_ctrl, &my_bs_udata);
	my_base_loop(my_ev_base->base);
	return 0;
}

