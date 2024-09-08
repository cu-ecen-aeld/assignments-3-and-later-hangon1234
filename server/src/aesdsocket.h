/* header for aesdsocket assignment */

#ifndef _aesdsocket
#define _aesdsocket

#include <signal.h>
#include <time.h>
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

/**
* set @param result with @param ts_1 + @param ts_2
*/
static inline void timespec_add( struct timespec *result,
                        const struct timespec *ts_1, const struct timespec *ts_2)
{
    result->tv_sec = ts_1->tv_sec + ts_2->tv_sec;
    result->tv_nsec = ts_1->tv_nsec + ts_2->tv_nsec;
    if( result->tv_nsec > 1000000000L ) {
        result->tv_nsec -= 1000000000L;
        result->tv_sec ++;
    }
}

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
