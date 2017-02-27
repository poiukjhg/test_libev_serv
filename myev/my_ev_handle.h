#ifndef MY_EV_HANDLE
#define MY_EV_HANDLE

typedef int( *read_handle_helper)(char* str);
typedef int( *write_handle_helper)(char* str, void* buf);

typedef struct r_buffer_cb_func{
	read_handle_helper r_handle_test(char* input_line);
	struct r_buffer_cb_func *next_func; 
	struct r_buffer_cb_func *head_func; 
}r_buffer_cb_func;

typedef struct w_buffer_cb_func{
	write_handle_helper w_handle_test(char* str, void* buf);
	struct w_buffer_cb_func *next_func; 
	struct w_buffer_cb_func *head_func;
}w_buffer_cb_func;

typedef struct buffer_cb_func{
	r_buffer_cb_func * r_func;
	w_buffer_cb_func * w_func;
}buffer_cb_func;

typedef struct my_event_base{
	struct event_base* base;
	buffer_cb_func cb_func;
}my_event_base;


#endif