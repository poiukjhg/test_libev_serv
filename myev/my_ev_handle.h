#ifndef MY_EV_HANDLE
#define MY_EV_HANDLE

#define CHECK_FD_IS_SET(BITMAP, FD) (BITMAP[FD/8]&(0x80>>(FD%8)))
#define SET_FD_INTO_BITMAP(BITMAP, FD) do { \
   	(BITMAP[FD/8]|= (0x80>>(FD%8))) ;\
	} while(0)
	
typedef struct read_userdata{
	void *outbuf ;
	char* response;
	int len;
	struct bufferevent *bev; 
	char if_close_fd;
	void *user_data_tail;
}read_userdata;

typedef int ( *send_reply_helper)(read_userdata *read_data);
typedef int ( *read_handle_helper)(char *str, size_t recv_len, send_reply_helper send_func, read_userdata *read_data);
typedef int ( *write_handle_helper)(char *str, void *buf);
typedef int ( *get_cur_status_helper)(char *str);
typedef void (*event_callback_helper)(int, short, void *);

typedef struct r_buffer_cb_func{
	read_handle_helper r_handle_test;
	struct r_buffer_cb_func *next_func; 
	struct r_buffer_cb_func *head_func; 
}r_buffer_cb_func;

typedef struct w_buffer_cb_func{
	write_handle_helper w_handle_test;
	struct w_buffer_cb_func *next_func; 
	struct w_buffer_cb_func *head_func;
}w_buffer_cb_func;

typedef struct buffer_cb_func{
	r_buffer_cb_func * r_func;
	w_buffer_cb_func * w_func;
	get_cur_status_helper get_cur_status;
}buffer_cb_func;

typedef struct my_event_base{
	struct event_base* base;
	struct event *cur_ev; 
	int close_fd; 
	buffer_cb_func cb_func;
	char fd_map[8192];	
}my_event_base;

typedef struct my_base_userdata{
	my_event_base* my_ev_base;
	int fd;
}my_base_userdata;

my_event_base *my_create_base();
void my_base_init_loop_func(my_event_base *base, event_callback_helper func, void* args);
void my_base_loop(void *base);
void my_r_handler_helper_addtail(r_buffer_cb_func **cb_func, read_handle_helper func);
void my_w_handler_helper_addtail(w_buffer_cb_func **cb_func, write_handle_helper func);
void my_get_cur_status_helper_set(buffer_cb_func **buf_cb_func, get_cur_status_helper func);
void *my_start_listen(int listen_fd, my_event_base * my_ev_base);
void my_stop_listen(void *event);

#endif
