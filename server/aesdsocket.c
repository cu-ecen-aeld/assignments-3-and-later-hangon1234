#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
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
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* use getaddrinfo to get addrinfo structure */
    ret = getaddrinfo(NULL, MYPORT, &hints, &addrinfo_ptr);

    /* Check retcode of getaddrinfo */
    if (ret != 0) {
        return(ret); // exit if ret != 0
    }

    /* Get socket_fd */
    int socket_fd = socket(addrinfo_ptr->ai_family, addrinfo_ptr->ai_socktype, addrinfo_ptr->ai_flags);

    /* check return value of socket
     * it returns fd if success
     * or -1 if on error
     */

    if (socket_fd == -1) { // socket() returns error
        return(socket_fd);
    }

    /* use bind to assign address */
    ret = bind(socket_fd, addrinfo_ptr->ai_addr, addrinfo_ptr->ai_addrlen);
    
    /* bind() also returns -1 on error. check error */
    if (ret == -1) {
        return(ret);
    }

    /* handling for incoming connection */
    ret = listen(socket_fd, BACKLOG);
    
    /* listen() also returns -1 on error. check error */
    if (ret == -1) {
        return(ret);
    }

    /* this structure will store information about the incoming connection 
     * to determine which host is calling from which port 
     */
    struct sockaddr_storage connection_info;

    /* size of the struct sockaddr_storage
     * accept() will not put more bytes than addr_size
     * If fewer, accetp() will change the value of addr_size
     */
    socklen_t addr_size = sizeof(connection_info);

    /* accept a connection on a socket */
    int accept_fd = accept(socket_fd, (struct sockaddr *)&connection_info, &addr_size); 

    /* accept() also returns -1 on error. check error */
    if (accept_fd == -1){
        return(accept_fd);
    }

    /* print information about the incoming connection */
    printf("Accept connection from %s\n", ((struct sockaddr_in*)&connection_info)->sin_addr);

    /* buffer to store received data */
    char buf[BUFLEN];

    /* receive from socket */
    int recv_len = recv(socket_fd, buf, BUFLEN, 0);
    printf("received: %d\n", recv_len);

    
    /* free allocated addrinfo structure */
    freeaddrinfo(addrinfo_ptr);

    /* return 0 to indicate success */
    return 0;
}
