#ifndef MY_EV_HANDLE
#define MY_EV_HANDLE

typedef struct send_reply_data{
	void *outbuf ;
	char* response;
	int len;
}send_reply_data;

typedef int ( *send_reply_helper)(send_reply_data *send_data);
typedef int ( *read_handle_helper)(char *str, size_t recv_len, send_reply_helper send_func, send_reply_data *send_data);
typedef int ( *write_handle_helper)(char *str, void *buf);
typedef int ( *get_cur_status_helper)(char *str);

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
	buffer_cb_func cb_func;
}my_event_base;

my_event_base *my_base_init();
void my_r_handler_helper_addtail(r_buffer_cb_func **cb_func, read_handle_helper func);
void my_w_handler_helper_addtail(w_buffer_cb_func **cb_func, write_handle_helper func);
void *my_start_listen(int listen_fd, my_event_base * my_ev_base);
void my_get_cur_status_helper_set(buffer_cb_func **buf_cb_func, get_cur_status_helper func);
#endif
