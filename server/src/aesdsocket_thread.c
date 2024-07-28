#include "aesdsocket.h"

extern bool EXIT_SIGNAL;

void get_client_info(struct sockaddr * p_sockaddr, socklen_t size, char* client_address) {

    // Determine ipv4 or ipv6
    if (p_sockaddr->sa_family == AF_INET) {
	// if ipv4
        struct sockaddr_in* p_sockaddr_in = (struct sockaddr_in*) p_sockaddr;
        inet_ntop(AF_INET,(const void*) &p_sockaddr_in->sin_addr, client_address, size); 
    } else {
	struct sockaddr_in6* p_sockaddr_in6;
        p_sockaddr_in6 = (struct sockaddr_in6*) p_sockaddr;
        inet_ntop(AF_INET6, (const void*) &p_sockaddr_in6->sin6_addr, client_address, size);
    }
    return;
}

// thread handling
void thread_socket_receive(thread_data* thread_param) {
    int ret = 0;

    syslog(LOG_USER, "Accepted connection from %s\n", thread_param->client_address);
    printf("Accepted connection from %s\n", thread_param->client_address);

    /* Allocate buffer for socket operation */
    char buf[BUFLEN];
    buf[BUFLEN] = '\0';

    /* receive until newline received */
    while(EXIT_SIGNAL == false) {
        /* receive over socket */
        ret = recv(thread_param->fd_accept, buf, BUFLEN-1, 0);
    
        /* recv() returns -1 on error; returns number of bytes actually read into the buffer */
        /* check if error is occured */
        if (ret == -1) {
            printf("recv() failed!\n");
            thread_param->thread_exit_status = false;
            return;
        }
    
        /* Write received message to the file */
        fwrite(buf, ret, 1, thread_param->fp); 

        /* send back received message to the client */
        if (strchr(buf, '\n') != NULL) {
            fflush(thread_param->fp);
            break;
        }
    }
}
