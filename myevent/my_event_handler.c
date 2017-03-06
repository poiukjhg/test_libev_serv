#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../mylogs.h"
#include "../mytype.h"
#include "mylock.h"
#include "my_event_handler.h"
#include <sys/wait.h>
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


void my_read_cb(struct bufferevent *bev, void *ctx)
{  
	struct evbuffer *input, *output;  
	char *request_line; 
	read_userdata *read_ud = (read_userdata *)ctx;
	my_base *my_bs =  read_ud->my_bs;
	base_listenfd_list *cur_listen_nod = my_bs->listen_fd_list; 
	size_t len;  
	char* recv_buf = NULL;
	size_t recv_len = 0;
	bufread_cb_list *rd_buf_cb_nod= NULL;
	read_handle_helper r_handler = NULL;

	//printf("read callback\n\r");
	while(cur_listen_nod->fd != read_ud->listen_fd){
		cur_listen_nod = cur_listen_nod->next;
		if (cur_listen_nod == NULL)
			return;
	}
	rd_buf_cb_nod = cur_listen_nod->rd_buf_cb_list;
		
	input = bufferevent_get_input(bev);
	output = bufferevent_get_output(bev);

	while(1) {  
		request_line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
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
	while(rd_buf_cb_nod != NULL){
		r_handler = rd_buf_cb_nod->r_handler;
		if(r_handler(recv_buf, recv_len, read_ud) == SEND_RESPONSE){
			evbuffer_add(output, read_ud->response, read_ud->send_len);
		}
		rd_buf_cb_nod = rd_buf_cb_nod->next_func;
	}
	read_ud->read_buf = input;		
	read_ud->write_buf = output;
freebuf: 
	if(request_line != NULL) {  
	        free(request_line);
		request_line = NULL;
	}	
	if(recv_buf != NULL) {  
	        free(recv_buf);
		recv_buf = NULL;
	}		
	//my_try_to_listen(my_bs);
}

void my_write_cb(struct bufferevent *bev, void *ctx)
{	
	read_userdata *read_ud = (read_userdata *)ctx;
	int fd = bufferevent_getfd(bev);	
	my_base *my_bs = read_ud->my_bs;
	if (read_ud->close_fd == 1 ){
		//printf("write callback pid = %d\n\r", (int)getpid());		
		bufferevent_free(bev);	
		free(ctx);
		close(fd);
		my_bs->accept_fd_num--;
	}
	my_try_to_listen(my_bs);
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
	my_base *my_bs =  (my_base *)arg;
	struct event_base *base = my_bs != NULL?my_bs->base:NULL;  
	struct sockaddr_storage ss;  
	struct bufferevent *bev = NULL;  	
	socklen_t slen = sizeof(ss);  
	read_userdata *read_ud = (read_userdata *)malloc(sizeof(read_userdata));
	if(read_ud == NULL){  
		handle_error("read_userdata");  
	}  
	memset(read_ud, 0, sizeof(read_userdata));
	int accept_fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);  
	//printf("accept\n\r");
	if (accept_fd < 0) {  
		handle_error("accept");  
	}  
	else  {  
		my_bs->accept_fd_num++;
		read_ud->my_bs = my_bs;
		read_ud->listen_fd = listen_fd;
		read_ud->accept_fd = accept_fd;
		evutil_make_socket_nonblocking(accept_fd);  
		bev = bufferevent_socket_new(base, (evutil_socket_t)accept_fd, BEV_OPT_CLOSE_ON_FREE); 
		
		if (!bev)  {
			handle_error("bufferevent new");
			return;   	
		}	
		bufferevent_setcb(bev, my_read_cb, my_write_cb, errorcb, (void*)read_ud);  
		bufferevent_enable(bev, EV_READ|EV_WRITE);
	}  	
	my_try_to_listen(my_bs);
}

void my_try_to_listen(my_base *my_bs)
{
	base_listenfd_list *cur_listen_fd = my_bs->listen_fd_list;
	struct event * ev;
	char res=RES_ERROR;
	if(my_bs->is_locked == 1){
		my_lock_tryunlock(my_bs->lock);		
	}
	
	if (my_bs->accept_fd_num>50){
		printf("pid = %d overload\n\r", (int)getpid());
		if(my_bs->is_locked == 1){
			while(cur_listen_fd){
				event_free(cur_listen_fd->ev);
				cur_listen_fd->ev = NULL;
				cur_listen_fd = cur_listen_fd->next;
			}
			my_bs->is_locked = 0;
		}
		return;
	}
	
	res = my_lock_trylock(my_bs->lock);
	if(res  == RES_ERROR){
		if(my_bs->is_locked == 1){
			//printf("pid = %d free event\n\r", (int)getpid());
			while(cur_listen_fd){
				event_free(cur_listen_fd->ev);
				cur_listen_fd->ev = NULL;
				cur_listen_fd = cur_listen_fd->next;
			}
			my_bs->is_locked = 0;
		}
	}
	else{
		if(my_bs->is_locked == 0){
			printf("pid = %d getlock\n\r", (int)getpid());
			while(cur_listen_fd){
				ev = event_new(my_bs->base, cur_listen_fd->fd, EV_READ|EV_PERSIST, my_accept_cb, (void*)my_bs);
				event_add(ev, NULL); 
				cur_listen_fd->ev = ev;
				cur_listen_fd = cur_listen_fd->next;
			}
			my_bs->is_locked = 1;
		}		
	}
}

void default_loop_callback_func(int fd, short event, void *arg)
{	
	my_base *my_bs = (my_base *)arg;	
	my_try_to_listen(my_bs);	
}

my_base *server_init()
{
	my_base *my_bs;	
	my_bs = (my_base *)malloc(sizeof(my_base));
	if (my_bs == NULL){
		handle_error("base init");
		return NULL;
	}
	memset(my_bs, 0, sizeof(my_base));		
	my_bs->base = event_base_new();
	if (my_bs->base == NULL)  {
		handle_error("ev base init");
		return NULL;   	
	}	
	
	return my_bs;
}	

void server_listen_fd_add(my_base *my_bs, int listen_fd)
{
	base_listenfd_list *cur_listen_nod = NULL;
	base_listenfd_list *tail_listen_nod = NULL;
	cur_listen_nod = (base_listenfd_list *)malloc(sizeof(base_listenfd_list));
	cur_listen_nod->fd = listen_fd;
	cur_listen_nod->ev = NULL;	
	cur_listen_nod->next = NULL;
	cur_listen_nod->rd_buf_cb_list = NULL;	
	if (my_bs->listen_fd_list == NULL){
		cur_listen_nod->head = cur_listen_nod;
		my_bs->listen_fd_list = cur_listen_nod;
	}
	else{
		tail_listen_nod = my_bs->listen_fd_list;
		cur_listen_nod->head = my_bs->listen_fd_list->head;
		while(tail_listen_nod->next != NULL)
			tail_listen_nod = tail_listen_nod->next;
		tail_listen_nod->next = cur_listen_nod;		
	}
}

void server_loop_cb_set(my_base *my_bs)
{
	struct event *my_timer_event;
	struct timeval tv = {0, 50};
	my_timer_event = event_new(my_bs->base, -1, EV_PERSIST, default_loop_callback_func, (void*)my_bs);
	evtimer_add(my_timer_event, &tv);	
}

void server_rfunc_add(my_base *my_bs, int fd, read_handle_helper func)
{
	base_listenfd_list *cur_listen_nod = my_bs->listen_fd_list;
	bufread_cb_list * read_buf_cb_node = NULL;
	bufread_cb_list * cur_read_buf_cb_node = NULL;
	while(cur_listen_nod != NULL){
		if (cur_listen_nod->fd == fd)
			break;
		cur_listen_nod = cur_listen_nod->next;
		if(cur_listen_nod == NULL)
			return ;		
	}

	read_buf_cb_node = (bufread_cb_list *)malloc(sizeof(bufread_cb_list));
	read_buf_cb_node->r_handler = func;
	read_buf_cb_node->next_func = NULL;
	cur_read_buf_cb_node = cur_listen_nod->rd_buf_cb_list;
	
	if (cur_read_buf_cb_node == NULL){
		cur_listen_nod->rd_buf_cb_list = read_buf_cb_node;
		cur_listen_nod->rd_buf_cb_list->head_func  = read_buf_cb_node;
		cur_listen_nod->rd_buf_cb_list->next_func  = NULL;
		return;
	}		
	while(cur_read_buf_cb_node->next_func != NULL)
		cur_read_buf_cb_node = cur_read_buf_cb_node->next_func;
	cur_read_buf_cb_node->next_func = read_buf_cb_node;
	read_buf_cb_node->head_func = cur_listen_nod->rd_buf_cb_list;	
}
void server_wfunc_add(my_base *my_bs, int fd, write_handle_helper func)
{

}
void server_start(my_base *my_bs)
{
	int  res;
	printf("loop start %d\n\r", (int)getpid());
	if (my_bs->base != NULL){
		res = event_base_dispatch(my_bs->base); 
		printf("loop back %d, res = %d\n\r", (int)getpid(), res);
	}
}
