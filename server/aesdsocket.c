#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define BUFLEN 128 
int main(void)
{
    /* socket server for assignment 5 part 1 */

    /* Get socket_fd */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* Get addrinfo structure */
    struct addrinfo * addrinfo_ptr = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));

    /* setup hints structure */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* use getaddrinfo to get addrinfo structure */
    getaddrinfo(NULL, "9000", &hints, &addrinfo_ptr);

    /* use bind to assign address */
    bind(socket_fd, addrinfo_ptr->ai_addr, sizeof(struct sockaddr));

    /* listen for connections on a socket */
    listen(socket_fd, 10);

    /* accept a connection on a socket */
    struct sockaddr * client_addr = NULL;
    socklen_t client_num = sizeof(struct sockaddr);
    int fd_rw = accept(socket_fd, client_addr, &client_num); 

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
