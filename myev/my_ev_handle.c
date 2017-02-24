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
		handle_error("base init");
		return RES_ERROR;   	
	}	
	return (void*)base;
}

void my_event_destroy(void *base){
	
}

void* my_base_event_ctrl(void *base, int fd, int flag, void* func){
	struct event *event;
	event = event_new(base, evutil_socket_t(fd), flag, func, (void*)base);
	event_add(event, NULL); 
	return (void*)event;
}

void* my_base_bufevent_ctrl(void *base, int fd, int flag, void *r_func, void *w_func, void *err_func){
	struct bufferevent *bev; 
	
	evutil_make_socket_nonblocking(fd);  
	bev = bufferevent_socket_new(base, (evutil_socket_t)fd, BEV_OPT_CLOSE_ON_FREE); 
	if (!bev)  {
		handle_error("bufferevent new");
		return RES_ERROR;   	
	}	
	bufferevent_setcb(bev, r_func, w_func, err_func, NULL);  
	bufferevent_enable(bev, flag);   	
	return (void*)bev;
}

void my_event_loop(void *base){
	event_base_dispatch(base); 
}



