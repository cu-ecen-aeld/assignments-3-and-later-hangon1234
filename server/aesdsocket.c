#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

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
    int ret = getaddrinfo(NULL, "1000", &hints, &addrinfo_ptr);

    /* use bind to assign address */
    int bind_ret = bind(socket_fd, addrinfo_ptr->ai_addr, sizeof(struct sockaddr));

    /* free allocated addrinfo structure */
    freeaddrinfo(addrinfo_ptr);

    /* return 0 to indicate success */
    return 0;
}
