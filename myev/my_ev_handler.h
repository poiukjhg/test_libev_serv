#ifndef MY_EVENT_HANDLE
#define MY_EVENT_HANDLE

#define SEND_RESPONSE 1

typedef struct bufread_cb_list bufread_cb_list;
typedef struct base_listenfd_list{
	int fd;
	ev_io *ev;
	bufread_cb_list *rd_buf_cb_list;
	struct base_listenfd_list *next; 
	struct base_listenfd_list *head; 
}base_listenfd_list;

typedef struct my_base{
	struct ev_loop *base;	
	char is_locked;
	void* lock;	
	int accept_fd_num;
	base_listenfd_list *listen_fd_list;	
}my_base;

typedef struct read_userdata{
	my_base* my_bs;
	void *read_buf ;
	void *write_buf ;	
	char* response;
	int listen_fd;
	int accept_fd;
	int recv_len;
	int send_len;	
	char close_fd;
}read_userdata;

typedef struct write_userdata{
	void *outbuf ;
	int len;
}write_userdata;


typedef void (*loop_cb_func)(my_base *my_bs, int fd);
typedef int ( *read_handle_helper)(char *recv_str, size_t recv_len, read_userdata *read_data);
typedef int ( *write_handle_helper)( write_userdata *write_data);

struct bufread_cb_list{
	read_handle_helper r_handler;
	struct bufread_cb_list *next_func; 
	struct bufread_cb_list *head_func; 
};

my_base *server_init();	
void server_listen_fd_add(my_base *my_bs, int listen_fd);
void my_try_to_listen(my_base *my_bs);
void server_loop_cb_set(my_base *my_bs);
void server_rfunc_add(my_base *my_bs, int fd, read_handle_helper func);
void server_wfunc_add(my_base *my_bs, int fd, write_handle_helper func);
void server_start(my_base *my_bs);

#endif
