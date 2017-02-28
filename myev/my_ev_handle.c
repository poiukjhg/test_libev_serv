#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/bufferevent.h>
#include <string.h>
#include "../mylogs.h"
#include "../mytype.h"
#include "my_ev_handle.h"
void* my_event_init()
{
	struct event_base* base;	
	base = event_base_new();
	if (!base)  {
		handle_error("ev base init");
		return NULL;   	
	}	
	return (void*)base;
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


my_event_base *my_base_init()
{
	my_event_base *my_ev_base;
	my_ev_base = (my_event_base *)malloc(sizeof(my_event_base));
	if (my_ev_base == NULL){
		handle_error("base init");
		return NULL;
	}
	memset(my_ev_base, 0, sizeof(my_event_base));
	if((my_ev_base->base = my_event_init()) == NULL){
		return NULL;
	}	
	return my_ev_base;
}

void my_event_destroy(my_event_base * my_ev_base)
{
	
}

void* my_base_event_ctrl(void *base, int fd, int flag, void (*func)(int, short, void *), void* args)
{
	struct event *event;
	event = event_new(base, fd, flag, func, (void*)args);
	event_add(event, NULL); 
	return (void*)event;
}

void* my_base_bufevent_ctrl(void *base, int fd, int flag,  bufferevent_data_cb r_func, bufferevent_data_cb w_func, void *err_func, void* cargs)
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

void my_event_loop(void *base)
{
	event_base_dispatch(base); 
}

void my_write_cb(struct bufferevent *bev, void *ctx)
{
	
}

void my_read_cb(struct bufferevent *bev, void *ctx)
{  
	struct evbuffer *input;  
	char *request_line;  
	buffer_cb_func *my_cb_func = (buffer_cb_func *)ctx;
	size_t len;  
	int index = -1;
	int flow_status = -1;	
	r_buffer_cb_func *r_func_list = my_cb_func->r_func->head_func;
	get_cur_status_helper get_cur_state = my_cb_func->get_cur_status;
	read_handle_helper r_func_handler = NULL;
		
	input = bufferevent_get_input(bev);

	while(1) {  
		request_line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
		if(request_line == '\0') {  
			goto freebuf;
		}  
		else{  
			flow_status = get_cur_state(request_line);
			for(index = 0; index < flow_status; index++){
				r_func_list = r_func_list->next_func;
			}
			r_func_handler = r_func_list->r_handle_test;
			r_func_handler(request_line);
		}    
	}  
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
	int accept_fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);  
	if (accept_fd < 0) {  
		handle_error("accept");  
	}  
	else  {  
		bev = my_base_bufevent_ctrl(base, accept_fd, EV_READ|EV_WRITE, my_read_cb, my_write_cb, errorcb, &(my_ev_base->cb_func));	
		if (bev == NULL){
			handle_error("bufferevent"); 
		}
	}  	
}

void* my_start_listen(int listen_fd, my_event_base * my_ev_base)
{
	struct event_base *base = my_ev_base->base;
	struct event *event;
	event = my_base_event_ctrl(base, listen_fd, EV_READ|EV_PERSIST, my_accept_cb, (void *)my_ev_base);
	my_event_loop(base);
	return (void*)event;
}


