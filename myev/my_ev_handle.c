#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include "../mylogs.h"

void* my_event_init()
{
	struct event_base* base;	
	base = event_base_new();
	if (!base)  {
		handle_error("ev base init");
		return RES_ERROR;   	
	}	
	return (void*)base;
}

void *my_r_handler_helper_set(r_buffer_cb_func *cb_func, read_handle_helper func){


}

void *my_w_handler_helper_set(w_buffer_cb_func *cb_func, write_handle_helper func){


}

void my_get_cur_status_helper_set(buffer_cb_func *buf_cb_func, get_cur_status_helper func){
	if (  buf_cb_func != NULL &&  func != NULL)
		buf_cb_func->get_cur_status = func;
}


my_event_base *my_base_init(){
	my_event_base *my_ev_base;
	my_ev_base = (my_ev_base *)malloc(sizeof(my_ev_base));
	if (my_ev_base == NULL){
		handle_error("base init");
		return RES_ERROR;
	}
	memset(my_ev_base, 0, sizeof(my_ev_base));
	if((my_ev_base->base = my_event_init()) == RES_ERROR){
		return RES_ERROR;
	}	
	return my_ev_base;
}

void my_event_destroy(my_event_base * my_ev_base){
	
}

void* my_base_event_ctrl(void *base, int fd, int flag, void (*func)(int, short, void *), void* args){
	struct event *event;
	event = event_new(base, evutil_socket_t(fd), flag, func, (void*)args);
	event_add(event, NULL); 
	return (void*)event;
}

void* my_base_bufevent_ctrl(void *base, int fd, int flag,  bufferevent_event_cb r_func, bufferevent_event_cb w_func, void *err_func, void* cargs){
	struct bufferevent *bev; 
	
	evutil_make_socket_nonblocking(fd);  
	bev = bufferevent_socket_new(base, (evutil_socket_t)fd, BEV_OPT_CLOSE_ON_FREE); 
	if (!bev)  {
		handle_error("bufferevent new");
		return RES_ERROR;   	
	}	
	bufferevent_setcb(bev, r_func, w_func, err_func, cargs);  
	bufferevent_enable(bev, flag);   	
	return (void*)bev;
}

void my_event_loop(void *base)
{
	event_base_dispatch(base); 
}

void my_write_cb(struct bufferevent *bev, void *ctx){
	
}

void my_read_cb(struct bufferevent *bev, void *ctx)
{  
	struct evbuffer *input, *output;  
	char *request_line;  
	size_t len;  
	int index = -1;
	int flow_status = -1;
	r_buffer_cb_func * r_func_list = (buffer_cb_func *)ctx->r_func->head;
	get_cur_status_helper get_cur_state = (buffer_cb_func *)ctx->get_cur_status;
	read_handle_helper r_func_handler = NULL;
		
	input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);
	while(1) {  
		request_line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
		if(request_line == '\0') {  
			goto freebuf;
		}  
		else{  
			flow_status = get_cur_state(request_line);
			for(index = 0; index < flow_status; index++){
				r_func_list = r_func_list->next;
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
    struct event_base *base = (my_event_base *)arg->base;  
    struct sockaddr_storage ss;  
    struct bufferevent *bev = NULL;  	
    socklen_t slen = sizeof(ss);  
    int accept_fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);  
    if (accept_fd < 0) {  
        handle_error("accept");  
    }  
    else  {  
	bev = my_base_bufevent_ctrl(base, accept_fd, EV_READ|EV_WRITE, my_read_cb, my_write_cb, errorcb, &((my_event_base *)arg->cb_func));
    }  	
}


