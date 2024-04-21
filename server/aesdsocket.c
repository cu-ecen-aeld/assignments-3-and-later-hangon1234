#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define BUFLEN 128 
#define BACKLOG 10 // maximum pending connections in the queue
#define MYPORT "9000"
#define TEMP_PATH "/var/tmp/aesdsocketdata"

// reference: https://beej.us/guide/bgnet/html/#socket

void send_file_to_client(int byte_written, int fd_accept) {
    FILE * fp = fopen(TEMP_PATH, "r");
    char buf[1];
    int read_size = 0;
    int i = 0;

    printf("Bytes to send: %d sizeofchar: %ld\n", byte_written, sizeof(char));
   
    /* send back to the client */
    for(i = 0; i < byte_written; i++) {
        read_size = fread(buf, 1, 1, fp);
        send(fd_accept, buf, read_size, 0);
    }

    return;
}

int log_client_info(struct sockaddr * p_sockaddr, socklen_t size) {
    char client_address[INET_ADDRSTRLEN];

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
    
    printf("Connection from address: %s\n", client_address);
    syslog(LOG_INFO, "Accepted connection from %s", client_address);

    return(1);
}

int main(void)
{
    /* socket server for assignment 5 part 1 */
    
    int ret = 0;

    /* Open file for saving received content */
    FILE * fp = fopen(TEMP_PATH, "w");
    /* File offeset, indicate position where returned to client */

    /* Check file opened or not */
    if (fp == NULL) {
        printf("fp is null, exit\n");
	return(1);
    }

    /* Get addrinfo structure */
    struct addrinfo * addrinfo_ptr = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));

    /* setup hints structure */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* use getaddrinfo to get addrinfo structure */
    ret = getaddrinfo(NULL, MYPORT, &hints, &addrinfo_ptr);

    /* Check retcode of getaddrinfo */
    if (ret != 0) {
	printf("getaddrinfo() failed!\n");
        return(ret); // exit if ret != 0
    }

    /* Get socket_fd */
    int socket_fd = socket(addrinfo_ptr->ai_family, addrinfo_ptr->ai_socktype, addrinfo_ptr->ai_protocol);

    /* check return value of socket
     * it returns fd if success
     * or -1 if on error
     */

    if (socket_fd == -1) { // socket() returns error
        printf("socket() failed! errno: %d\n", errno);
        return(socket_fd);
    }

    /* to fix bind() address already in use */
    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt error");
	exit(1);
    }

    /* use bind to assign address */
    ret = bind(socket_fd, addrinfo_ptr->ai_addr, addrinfo_ptr->ai_addrlen);
    
    /* bind() also returns -1 on error. check error */
    if (ret == -1) {
	printf("bind() failed!\n");
        return(ret);
    }

    /* free allocated addrinfo structure */
    freeaddrinfo(addrinfo_ptr);

    /* handling for incoming connection */
    ret = listen(socket_fd, 10);
    
    /* listen() also returns -1 on error. check error */
    if (ret == -1) {
	printf("listen() failed!\n");
        return(ret);
    }

    /* bytes written to the file */
    int byte_written = 0;
    int fd_accept; 
    /* accept a connection on a socket */
    struct sockaddr_storage client_addr;
    socklen_t client_num = sizeof(struct sockaddr);

    /* main accept loop */
    while (1) {
        fd_accept = accept(socket_fd, (struct sockaddr*)&client_addr, &client_num);

        /* accept() also returns -1 on error. check error */
        if (ret == -1) {
            printf("accept() failed!\n");
            return(ret);
        }

        /* Log connected client information */
        ret = log_client_info((struct sockaddr*)&client_addr, client_num);

        /* Allocate buffer for socket operation */
        char buf[BUFLEN];
        buf[BUFLEN] = '\0';

        /* receive until newline received */
	while(1) {
            /* receive over socket */
            ret = recv(fd_accept, buf, BUFLEN-1, 0);
     
            printf("buffer received: %s\n", buf);
            /* recv() returns -1 on error; returns number of bytes actually read into the buffer */
            /* check if error is occured */
            if (ret == -1) {
                printf("recv() failed!\n");
                return(ret);
            }
     
            /* send back received message to the client */
            printf("len: %d\n", ret);
            int i=0;
            for(i = 0; i < ret; i++)
            {
                printf("%c ", buf[i]);
            }
            printf("\n-------------\n");

            /* Write received message to the file */
            fwrite(buf, ret, 1, fp); 

            /* Increase received size */
            byte_written += ret;
	    
            /* send back received message to the client */
	    if (strchr(buf, '\n') != NULL) {
		fflush(fp);
	        send_file_to_client(byte_written, fd_accept);
	        break;
            }
        }
    }
    
    /* close file descriptor */
    fclose(fp);
    shutdown(fd_accept, 2);
    shutdown(socket_fd, 2);
    close(socket_fd);
    close(fd_accept);

    /* return 0 to indicate success */
    return 0;
}
