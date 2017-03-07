#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <unistd.h>
#include "../mylogs.h"
#include "../mytype.h"
#include "my_ev_handle.h"

void default_callback_func(int listen_fd, short event, void *arg)
{
	return;
}

char* relloc_buf(char* old_buf, char* new_str, int old_len, int new_len, char end_flag)
{
	char *new_buf = (char *)malloc(old_len+new_len+1);
	if (new_buf == NULL)
		return NULL;
	memset(new_buf, 0, old_len+new_len+1);
	memcpy(new_buf, old_buf, old_len);
	memcpy(new_buf+old_len, new_str, new_len);
	new_buf[old_len+new_len] = end_flag;
	return new_buf;
}

void* my_event_init(struct event_base* base)
{
	return NULL;
}

void my_event_destroy(struct event *event)
{
	
}

void my_r_handler_helper_addtail(r_buffer_cb_func **cb_func, read_handle_helper func)
{
	r_buffer_cb_func *phead_func = NULL;
	r_buffer_cb_func *pcur_func = NULL;
	r_buffer_cb_func *ptmp_func = NULL;
	if (*cb_func == NULL){
		*cb_func = (r_buffer_cb_func *)malloc(sizeof(r_buffer_cb_func));
		phead_func = *cb_func;
		phead_func->head_func = phead_func;
		phead_func->next_func = NULL;
		phead_func->r_handle_test = func;		
	}
	else{
		phead_func = *cb_func;
		phead_func = phead_func->head_func;
		pcur_func = phead_func;
		while(pcur_func->next_func != NULL){
			pcur_func = pcur_func->next_func;
		}
		ptmp_func = (r_buffer_cb_func *)malloc(sizeof(r_buffer_cb_func));
		ptmp_func->head_func = phead_func;
		ptmp_func->next_func = NULL;
		ptmp_func->r_handle_test = func;
		pcur_func->next_func = ptmp_func;
	}
}

void my_w_handler_helper_addtail(w_buffer_cb_func **cb_func, write_handle_helper func)
{
	w_buffer_cb_func *phead_func = NULL;
	w_buffer_cb_func *pcur_func = NULL;
	w_buffer_cb_func *ptmp_func = NULL;
	if (*cb_func == NULL){
		*cb_func = (w_buffer_cb_func *)malloc(sizeof(w_buffer_cb_func));
		phead_func = *cb_func;
		phead_func->head_func = phead_func;
		phead_func->next_func = NULL;
		phead_func->w_handle_test = func;		
	}
	else{
		phead_func = *cb_func;
		phead_func = phead_func->head_func;
		pcur_func = phead_func;
		while(pcur_func->next_func != NULL){
			pcur_func = pcur_func->next_func;
		}
		ptmp_func = (w_buffer_cb_func *)malloc(sizeof(w_buffer_cb_func));
		ptmp_func->head_func = phead_func;
		ptmp_func->next_func = NULL;
		ptmp_func->w_handle_test = func;
		pcur_func->next_func = ptmp_func;
	}
}

void my_get_cur_status_helper_set(buffer_cb_func **buf_cb_func, get_cur_status_helper func)
{
	buffer_cb_func * my_buffer_cb_func = *buf_cb_func ;
	if (  my_buffer_cb_func != NULL &&  func != NULL)
		my_buffer_cb_func->get_cur_status = func;
}

void my_base_loop(void *base)
{
	if (base != NULL){
		event_base_dispatch(base); 
	}
}

my_event_base *my_create_base()
{
	my_event_base *my_ev_base;
	//buffer_cb_func *my_cb_func;
	
	my_ev_base = (my_event_base *)malloc(sizeof(my_event_base));
	if (my_ev_base == NULL){
		handle_error("base init");
		return NULL;
	}
	memset(my_ev_base, 0, sizeof(my_event_base));		
	my_ev_base->base = event_base_new();
	if (my_ev_base->base == NULL)  {
		handle_error("ev base init");
		return NULL;   	
	}	
	//my_cb_func = &(my_ev_base->cb_func);
	//my_cb_func->r_func = NULL;
	//my_base_loop(my_ev_base->base);
	return my_ev_base;
}

void my_base_init_loop_func(my_event_base *my_ev_base, event_callback_helper func, void* args)
{
	struct event *my_timer_event;
	struct timeval tv = {0, 100};
	event_callback_helper cb_func = (func !=NULL)?func:default_callback_func;
	my_timer_event = event_new(my_ev_base->base, -1, EV_PERSIST, cb_func, (void*)args);
	evtimer_add(my_timer_event, &tv);	
}

void* my_base_fd_event_add(void *base, int fd, int flag, event_callback_helper func, void* args)
{
	struct event *event;
	event = event_new(base, fd, flag, func, (void*)args);
	event_add(event, NULL); 
	return (void*)event;
}

void my_base_fd_event_remove(struct event *event)
{
	event_free(event);
	return;
}



void* my_base_bufevent_new(void *base, int fd, int flag,  bufferevent_data_cb r_func, bufferevent_data_cb w_func, void *err_func, void* cargs)
{
	struct bufferevent *bev; 
	
	evutil_make_socket_nonblocking(fd);  
	bev = bufferevent_socket_new(base, (evutil_socket_t)fd, BEV_OPT_CLOSE_ON_FREE); 
	if (!bev)  {
		handle_error("bufferevent new");
		return NULL;   	
	}	
	bufferevent_setcb(bev, r_func, w_func, err_func, cargs);  
	bufferevent_enable(bev, flag);   	
	return (void*)bev;
}

void my_base_bufevent_del(struct bufferevent *bev)
{
	bufferevent_free(bev);
}

void my_write_cb(struct bufferevent *bev, void *ctx)
{	
	my_event_base *my_ev_base =  (my_event_base *)ctx;
	int fd = bufferevent_getfd(bev);
	if (my_ev_base->close_fd == fd ){
		printf("write callback\n\r");		
		my_base_bufevent_del(bev);
		close(fd);
		my_ev_base->close_fd = 0;
	}
}

int my_send_by_buffer(read_userdata *read_data)
{
	struct evbuffer *outbuf = (struct evbuffer *)(read_data->outbuf);
	struct bufferevent *bev = read_data->bev;
	my_event_base *my_ev_base =  (my_event_base *)read_data->user_data_tail;
	evbuffer_add(outbuf, read_data->response, read_data->len);
	if (read_data->if_close_fd == 1){
		my_ev_base->close_fd = bufferevent_getfd(bev);
	}
	return read_data->len;
}

void my_read_cb(struct bufferevent *bev, void *ctx)
{  
	//printf("read callback\n\r");
	struct evbuffer *input, *output;  
	char *request_line;  
	my_event_base *my_ev_base =  (my_event_base *)ctx;
	buffer_cb_func *my_cb_func =&(my_ev_base->cb_func);
	size_t len;  
	int index = -1;
	int flow_status = -1;	
	read_userdata read_data = {0};
	char* recv_buf = NULL;
	size_t recv_len = 0;
	
	if (my_cb_func == NULL){
		printf("%s:%d\n\r", __FILE__, __LINE__);
		exit(-1);
	}
	r_buffer_cb_func *r_func_list = my_cb_func->r_func->head_func;
	get_cur_status_helper get_cur_state = my_cb_func->get_cur_status;
	read_handle_helper r_func_handler = NULL;
		
	input = bufferevent_get_input(bev);
	output = bufferevent_get_output(bev);

	while(1) {  
		request_line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
		//printf("recv len:%d, %s\n\r", len, request_line);
		if (strlen(request_line) == 0){
			if(recv_len == 0){
				printf("recv NULL\n\r");
				goto freebuf;
			}
			else
				break;			
		}	
		recv_buf = relloc_buf(recv_buf, request_line, recv_len, len, '|');
		if(request_line != NULL) {  
		        free(request_line);
			request_line = NULL;
		}		
		recv_len = recv_len+len+1;  		
	}  
	flow_status = get_cur_state(recv_buf);
	for(index = 0; index < flow_status; index++){
		r_func_list = r_func_list->next_func;
	}
	r_func_handler = r_func_list->r_handle_test;
	read_data.outbuf = output;		
	read_data.bev = bev;
	read_data.user_data_tail = (void *)my_ev_base;
	r_func_handler(recv_buf, recv_len, my_send_by_buffer, &read_data);	
freebuf: 
	if(request_line != NULL) {  
	        free(request_line);
		request_line = NULL;
	}	
	
}

void errorcb(struct bufferevent *bev, short error, void *ctx)  
{  
	if (error & BEV_EVENT_EOF) {  
	/* connection has been closed, do any clean up here */  
		printf("connection closed\n");  
	}  
	else if (error & BEV_EVENT_ERROR){  
	/* check errno to see what error occurred */  
		printf("some other error\n");  
	}  
	else if (error & BEV_EVENT_TIMEOUT)  {  
	/* must be a timeout event handle, handle it */  
		printf("Timed out\n");  
	}  
	bufferevent_free(bev);  
} 


void my_accept_cb(int listen_fd, short event, void *arg)
{
	my_event_base *my_ev_base =  (my_event_base *)arg;
	struct event_base *base = my_ev_base != NULL?my_ev_base->base:NULL;  
	struct sockaddr_storage ss;  
	struct bufferevent *bev = NULL;  	
	socklen_t slen = sizeof(ss);  
	if (base == NULL){
		return;
	}
	int accept_fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);  
	if (accept_fd < 0) {  
		handle_error("accept");  
	}  
	else  {  
		bev = my_base_bufevent_new(base, accept_fd, EV_READ|EV_WRITE, my_read_cb, my_write_cb, errorcb, my_ev_base);	
		if (bev == NULL){
			handle_error("bufferevent"); 
		}
	}  	
}

void* my_start_listen(int listen_fd, my_event_base * my_ev_base)
{
	struct event_base *base = my_ev_base->base;
	struct event *event;
	if(CHECK_FD_IS_SET(my_ev_base->fd_map, listen_fd) >0){
		//printf("already add listen fd\n\r");
		return NULL;
	}
	event = my_base_fd_event_add(base, listen_fd, EV_READ|EV_PERSIST, my_accept_cb, (void *)my_ev_base);	
	SET_FD_INTO_BITMAP(my_ev_base->fd_map, listen_fd);	
	return (void*)event;
}

void my_stop_listen(void *event)
{
	my_base_fd_event_remove((struct event *)event);
	return;
}

