/* header for aesdsocket assignment */

#ifndef _aesdsocket
#define _aesdsocket

#include <time.h>
#include <signal.h>
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
#include <errno.h>
#include <syslog.h>
#include "queue.h"
#define BUFLEN 128 
#define BACKLOG 10 // maximum pending connections in the queue
#define MYPORT "9000"
#define TEMP_PATH "/var/tmp/aesdsocketdata"
#define RETCODE_SUCCESS 0
#define RETCODE_FAILURE 1

typedef struct thread_data {
    bool thread_exit; // true if terminated
    pthread_mutex_t * mutex;
    int fd_accept;
    char client_address[INET_ADDRSTRLEN];
    FILE* fp;
} thread_data;

typedef struct node {
    pthread_t thread;
    bool thread_exit;
    TAILQ_ENTRY(node) nodes;
} node_t;

typedef TAILQ_HEAD(head_s, node) head_t;

void thread_socket_receive(thread_data* thread_param);
void get_client_info(struct sockaddr * p_sockaddr, socklen_t size, char* client_address);
void check_thread_exit(head_t * head); 
void release_all_thread(head_t * head);
void timer_thread(union sigval sigval);

#endif /* _aesdsocket */
