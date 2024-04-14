#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define BUFLEN 128 
#define BACKLOG 10 // maximum pending connections in the queue
#define MYPORT "9000"

// reference: https://beej.us/guide/bgnet/html/#socket

int main(void)
{
    /* socket server for assignment 5 part 1 */
    
    int ret = 0;

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

    /* accept a connection on a socket */
    struct sockaddr * client_addr = NULL;
    socklen_t client_num = sizeof(struct sockaddr);
    int fd_accept = accept(socket_fd, client_addr, &client_num); 

    /* accept() also returns -1 on error. check error */
    if (ret == -1) {
	printf("accept() failed!\n");
        return(ret);
    }

    /* Allocate buffer for socket operation */
    char buf[BUFLEN];

    /* receive over socket */
    ret = recv(fd_accept, buf, BUFLEN, 0);

    printf("buffer received: %s\n", buf);
    /* recv() returns -1 on error; returns number of bytes actually read into the buffer */
    /* check if error is occured */
    if (ret == -1) {
	printf("recv() failed!\n");
        return(ret);
    }

    /* send back received message to the client */
    buf[BUFLEN] = '\0';
    int len = strlen(buf);
    printf("len: %d\n", len);
    printf("buffer: %s\n", buf);
    printf("=====\n");
    int bytes_sent = send(fd_accept, buf, len , 0);
    
    printf("bytes sent: %d\n", bytes_sent);


    /* close file descriptor */
    shutdown(fd_accept, 2);
    shutdown(socket_fd, 2);
    close(socket_fd);
    close(fd_accept);

    /* return 0 to indicate success */
    return 0;
}
