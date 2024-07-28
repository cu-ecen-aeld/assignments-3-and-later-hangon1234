/* header for aesdsocket assignment */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#define BUFLEN 128 
#define BACKLOG 10 // maximum pending connections in the queue
#define MYPORT "9000"
#define TEMP_PATH "/var/tmp/aesdsocketdata"
#define RETCODE_SUCCESS 0
#define RETCODE_FAILURE 1

typedef struct thread_data {
    bool thread_exit_status; // true if success, false if failed
    bool thread_stop;
    pthread_mutex_t * mutex;
    int fd_accept;
    char client_address[INET_ADDRSTRLEN];
    FILE* fp;
} thread_data;

void thread_socket_receive(thread_data* thread_param);
void get_client_info(struct sockaddr * p_sockaddr, socklen_t size, char* client_address);
