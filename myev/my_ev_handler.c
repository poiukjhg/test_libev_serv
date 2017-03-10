#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>  
#include <stddef.h> 
#include <fcntl.h>
#include "../mylogs.h"
#include "../mytype.h"
#include "mylock.h"
#include "my_ev_handler.h"

/*
#define my_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({ \
	const typeof(((type *) 0)->member) *__mptr = (ptr); \
        (type *) ((char *)__mptr - my_offsetof(type, member));}) 


#define my_container_of(ptr, type, member) ({ (type *) ((char *)ptr - offsetof(type, member));})
*/

#define setNonblock(fd) do{ \
	int flags = fcntl(fd, F_GETFL, 0); \
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);\
}while(0)

#define BUFFER_SIZE 8192

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


void my_read_cb(EV_P, ev_io *w, int revents)
{  
	read_userdata *read_ud = (read_userdata *)w->data;
	my_base *my_bs = read_ud->my_bs;
	char *read_buf = NULL;
	char *tmp_read_buf = (char *)malloc(BUFFER_SIZE/2);
	char *write_buf = NULL;
	char *tmp_write_buf = NULL;
	int read_len = 0;
	int tmp_read_len = 0;
	ssize_t write_len = 0;
	ssize_t tmp_write_len = 0;	
	base_listenfd_list *cur_listen_nod = my_bs->listen_fd_list; 
	bufread_cb_list *rd_buf_cb_nod= NULL;
	read_handle_helper r_handler = NULL;
	
	if (tmp_read_buf == NULL)
		handle_error("malloc");
	while(true){
		memset(tmp_read_buf, '0', BUFFER_SIZE/2);
		tmp_read_len = read(w->fd, tmp_read_buf, BUFFER_SIZE/2);
		if (tmp_read_len < 0 ){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				free(tmp_read_buf);
				tmp_read_buf = NULL;				
				break;
			}
			else
				handle_error("read");
		}
		else if(tmp_read_len == 0){
			free(tmp_read_buf);
			tmp_read_buf = NULL;
			break;
		}
		else{
			read_buf = relloc_buf(read_buf, tmp_read_buf, read_len, tmp_read_len, '|');
			read_len = read_len+tmp_read_len;
		}
	}
	log_output("recv from fd %d\n\r", w->fd);
	while(cur_listen_nod->fd != read_ud->listen_fd){
		cur_listen_nod = cur_listen_nod->next;
		if (cur_listen_nod == NULL)
			return;
	}
	rd_buf_cb_nod = cur_listen_nod->rd_buf_cb_list;	
	while(rd_buf_cb_nod != NULL){
		r_handler = rd_buf_cb_nod->r_handler;
		if(r_handler(read_buf, read_len, read_ud) == SEND_RESPONSE){
			write_len = read_ud->send_len;
			write_buf = read_ud->response;
			tmp_write_buf = write_buf;
			while(true){
				tmp_write_len = write(w->fd, tmp_write_buf, write_len);
				if(tmp_write_len <0){
					if(errno == EAGAIN || errno ==EWOULDBLOCK)
						continue;
					else
						handle_error("write");
				}
				else if(write_len == 0){
					//free(write_buf);					
					break;
				}
				else{
					write_len = write_len -tmp_write_len;
					tmp_write_buf = tmp_write_buf +tmp_write_len;
				}
			}
		}
		rd_buf_cb_nod = rd_buf_cb_nod->next_func;
	}	
}

void my_write_cb(EV_P, ev_io *w, int revents)
{	
	read_userdata *read_ud = (read_userdata *)w->data;
	if (read_ud->close_fd == 1){
		log_output("close accept fd %d, listen fd %d\n\r", read_ud->accept_fd, read_ud->listen_fd);
		close(read_ud->accept_fd);
		ev_io_stop (read_ud->my_bs->base, read_ud->w_ev);
		free(read_ud->w_ev);
		ev_io_stop (read_ud->my_bs->base, read_ud->r_ev);		
		free(read_ud->r_ev);
		free(read_ud);
	}
	

}

void errorcb(EV_P, ev_io *w, int revents)
{  

} 

void my_accept_cb(EV_P, ev_io *w, int revents)
{
	my_base *my_bs = w->data;
	ev_io *r_ev = (ev_io *)malloc(sizeof(ev_io));;
	ev_io *w_ev =  (ev_io *)malloc(sizeof(ev_io));;
	struct sockaddr_storage ss;   	
	socklen_t slen = sizeof(ss);  
	read_userdata *read_ud = (read_userdata *)malloc(sizeof(read_userdata));
	if(read_ud == NULL){  
		handle_error("read_userdata");  
	}  
	memset(read_ud, 0, sizeof(read_userdata));
	int accept_fd = accept(w->fd, (struct sockaddr*)&ss, &slen);  
	if (accept_fd < 0) {  		 
		if (accept_fd == EAGAIN || accept_fd == EWOULDBLOCK){
			return;
		}
		else
			handle_error("accept"); 
	}  
	else  {  
		log_output("accept fd %d, listen fd %d\n\r", accept_fd, w->fd);
		setNonblock(accept_fd);
		my_bs->accept_fd_num++;
		read_ud->my_bs = my_bs;
		read_ud->listen_fd = w->fd;
		read_ud->accept_fd = accept_fd;
		read_ud->r_ev = r_ev;
		read_ud->w_ev = w_ev;		
		r_ev->data = read_ud;
		w_ev->data = read_ud;
		//read_ud->close_fd = 1;
		ev_io_init (r_ev, my_read_cb,  accept_fd, EV_READ);
		ev_io_init (w_ev, my_write_cb,  accept_fd, EV_WRITE);
		ev_io_start (my_bs->base, r_ev);		
		ev_io_start (my_bs->base, w_ev);		
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
		log_output("pid = %d overload\n\r", (int)getpid());
		if(my_bs->is_locked == 1){
			while(cur_listen_fd){
				log_output("close listen fd %d\n\r", cur_listen_fd->fd);
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
			//log_output("pid = %d free event\n\r", (int)getpid());
			while(cur_listen_fd){	
				log_output("close listen fd %d\n\r", cur_listen_fd->fd);
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
			log_output("pid = %d getlock\n\r", (int)getpid());
			while(cur_listen_fd){	
				ev = (ev_io *)malloc(sizeof(ev_io));
				ev->data = my_bs;
				ev_io_init (ev, my_accept_cb,  cur_listen_fd->fd, EV_READ);
				ev_io_start (my_bs->base, ev);
				cur_listen_fd->ev = ev;
				log_output("listen fd %d\n\r", cur_listen_fd->fd);
				cur_listen_fd = cur_listen_fd->next;				
			}
			my_bs->is_locked = 1;
		}		
	}
}

void default_loop_callback_func(EV_P, ev_periodic *w, int revents)
{	
	my_base *my_bs ;
	//my_bs = my_container_of(loop, my_base, base);	
	my_bs = w->data; 
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
/*
	ev_timer *my_timer_ev;
	
	ev_init (my_timer_ev, default_loop_callback_func);
	my_timer_ev->repeat = 0.001;
	ev_timer_again (my_bs->base, my_timer_ev);		
*/
	ev_periodic *my_timer_ev = (ev_periodic *)malloc(sizeof(ev_periodic));
	my_timer_ev->data = my_bs;
	ev_periodic_init (my_timer_ev, default_loop_callback_func, 0., 1., 0);
	ev_periodic_start (my_bs->base, my_timer_ev);	
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
	log_output("loop start %d\n\r", (int)getpid());
	if (my_bs->base != NULL){
		res = ev_run (my_bs->base, 0);
		log_output("loop back %d, res = %d\n\r", (int)getpid(), res);
	}
}
