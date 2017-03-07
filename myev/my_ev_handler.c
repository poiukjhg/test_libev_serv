#include <ev.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>  
#include <stddef.h> 
#include "../mylogs.h"
#include "../mytype.h"
#include "mylock.h"
#include "my_ev_handler.h"
 
  
#define container_of(ptr, type, member) ({                      \  
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \  
        (type *) ((char *) __mptr - offsetof(type, member));}) 
        

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


void my_read_cb(EV_P_ ev_io *w, int revents)
{  

}

void my_write_cb(EV_P_ ev_io *w, int revents)
{	

}

void errorcb(struct bufferevent *bev, short error, void *ctx)  
{  

} 

void my_accept_cb(EV_P_ ev_io *w, int revents)
{
	my_base *my_bs = container_of(EV_P_, my_base, base);
	struct event_base *base = my_bs != NULL?my_bs->base:NULL;  
	struct sockaddr_storage ss;   	
	socklen_t slen = sizeof(ss);  
	read_userdata *read_ud = (read_userdata *)malloc(sizeof(read_userdata));
	if(read_ud == NULL){  
		handle_error("read_userdata");  
	}  
	memset(read_ud, 0, sizeof(read_userdata));
	int accept_fd = accept(my_bs->listen_fd, (struct sockaddr*)&ss, &slen);  
	//printf("accept\n\r");
	if (accept_fd < 0) {  		 
		if (accept_fd == EAGAIN or accept_fd == EWOULDBLOCK){
			return;
		}
		else
			handle_error("accept"); 
	}  
	else  {  
		my_bs->accept_fd_num++;
		read_ud->my_bs = my_bs;
		read_ud->listen_fd = listen_fd;
		read_ud->accept_fd = accept_fd;
		ev_io_init (ev, my_read_cb,  accept_fd, EV_READ|EV_PERSIST);
		ev_io_init (ev, my_write_cb,  accept_fd, EV_READ|EV_PERSIST);
		ev_io_start (my_bs->base, ev);		
	}  	
	my_try_to_listen(my_bs);
}

void my_try_to_listen(my_base *my_bs)
{
	base_listenfd_list *cur_listen_fd = my_bs->listen_fd_list;
	char res=RES_ERROR;
	ev_io *ev;
	if(my_bs->is_locked == 1){
		my_lock_tryunlock(my_bs->lock);		
	}
	
	if (my_bs->accept_fd_num>50){
		printf("pid = %d overload\n\r", (int)getpid());
		if(my_bs->is_locked == 1){
			while(cur_listen_fd){
				ev_io_stop (my_bs->base, cur_listen_fd->ev);
				free(cur_listen_fd->ev);
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
				ev_io_stop (my_bs->base, cur_listen_fd->ev);
				free(cur_listen_fd->ev);
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
				ev_io_init (ev, my_accept_cb,  cur_listen_fd->fd, EV_READ|EV_PERSIST);
				ev_io_start (my_bs->base, ev);
				cur_listen_fd->ev = ev;
				cur_listen_fd = cur_listen_fd->next;
			}
			my_bs->is_locked = 1;
		}		
	}
}

void default_loop_callback_func(EV_P_ ev_timer *w, int revents)
{		
	my_base *my_bs = container_of(EV_P_, my_base, base);	
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
	my_bs->base = EV_DEFAULT;
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
	ev_timer *my_timer_ev;
	ev_timer_init (my_timer_ev, default_loop_callback_func, 20, 0);
	ev_timer_start (my_bs->base, my_timer_ev);	
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
		ev_run (my_bs->base, 0);
		printf("loop back %d, res = %d\n\r", (int)getpid(), res);
	}
}
