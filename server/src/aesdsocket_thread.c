#include "aesdsocket.h"
#include "queue.h"

extern bool EXIT_SIGNAL;

void send_file_to_client(int fd_accept) {
    FILE * fp = fopen(TEMP_PATH, "r");
    int ch;

    /* send back to the client */
    while ((ch = fgetc(fp)) != EOF) {
        send(fd_accept, &ch, 1, 0);
    }
                
    fclose(fp);

    return;
}

void check_thread_exit(head_t * head) {
    node_t * p_node;
    TAILQ_FOREACH(p_node, head, nodes)
    {
        if (p_node->thread_exit == true) {
            // Thread is terminated
            pthread_join(p_node->thread, NULL);
        }
    }
    return;
}

void release_all_thread(head_t * head) {
    struct node * p_node;
    while(!TAILQ_EMPTY(head)) {
        p_node = TAILQ_FIRST(head);
        pthread_join(p_node->thread, NULL);
        TAILQ_REMOVE(head, p_node, nodes);
        free(p_node);
    }

}


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
    int byte_written = 0;

    syslog(LOG_USER, "Accepted connection from %s\n", thread_param->client_address);
    printf("Accepted connection from %s\n", thread_param->client_address);

    /* Allocate buffer for socket operation */
    char buf[BUFLEN];
    buf[BUFLEN] = '\0';

    /* get a lock to prevent interleaving output from timer thread */
    pthread_mutex_lock(thread_param->mutex);

    /* receive until newline received */
    while(EXIT_SIGNAL == false) {

        /* receive over socket */
        ret = recv(thread_param->fd_accept, buf, BUFLEN-1, 0);
    
        /* recv() returns -1 on error; returns number of bytes actually read into the buffer */
        /* check if error is occured */
        if (ret == -1) {
            printf("recv() failed!\n");
            thread_param->thread_exit = true;
            pthread_mutex_unlock(thread_param->mutex);
            pthread_exit(0);
        }
        
        /* Write received message to the file */
        fwrite(buf, ret, 1, thread_param->fp); 
        byte_written += ret;

        if (strchr(buf, '\n') != NULL) {
            fflush(thread_param->fp);
            break;
        }
    }

    /* Send received data back to client */
    send_file_to_client(thread_param->fd_accept);

    printf("Send back to client!\n");
    /* close file descriptor */
    shutdown(thread_param->fd_accept, 2);
    close(thread_param->fd_accept);

    /* release mutex */
    pthread_mutex_unlock(thread_param->mutex);

    /* exit thread */
    thread_param->thread_exit = true;
    pthread_exit(0);

    return;
}
