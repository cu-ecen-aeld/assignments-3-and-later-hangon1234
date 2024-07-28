#define _POSIX_C_SOURCE 200112L

#include "aesdsocket.h"

// reference: https://beej.us/guide/bgnet/html/#socket

// will be TRUE if SIGINT or SIGTERM is received
bool EXIT_SIGNAL;



static void sigint_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        syslog(LOG_USER, "Caught signal, exiting\n");
	// enable EXIT_SIGNAL to true to gracefully terminate the program
	EXIT_SIGNAL = true;
    } else {
        // this should not happen
	exit(EXIT_FAILURE);
    }
}

int main(int argc, char ** argv)
{
    /* socket server for assignment 5 part 1 */
    
    /* Initialize global variable */
    EXIT_SIGNAL = 0;

    /* initialize mutex */
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Variable to enable daemon mode */
    int enable_daemon = 0;

    /* register signal_handler */
    if (signal(SIGTERM, sigint_handler) == SIG_ERR) {
        printf("SIGTERM register failed!\n");
	exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
         printf("SIGINT register failed!\n");
	 exit(EXIT_FAILURE);
    }

    /* get option */
    int c;
    while((c = getopt(argc, argv, "d")) != -1) {
        switch(c){
	        case 'd':
        	    enable_daemon = 1; 
        	    break;
            default:
                printf("Please check option\n");
                exit(1);
        }
    }

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

    /* port 9000 is available. Fork if daemon is enabled */
    if (enable_daemon) {
        int pid;
        pid = fork();
        
	if (pid == -1) // error occured
	{
	    printf("Failed to fork, exit!\n");
	    syslog(LOG_PERROR, "Failed to fork, exit\n");
	    exit(RETCODE_FAILURE);
	} else if (pid == 0) // child process, datach from parent
        {
            printf("This is child process, continue..\n");
            if (setsid() == -1) {
	    }
	} else { 
        // this is parent process
	    // exit with success
	    exit(RETCODE_SUCCESS);
        }
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

    /* file descriptor */
    int fd_accept = 0; 
    socklen_t client_num = sizeof(struct sockaddr);

    /* Initialize linked list */
    head_t head;
    TAILQ_INIT(&head);

    /* main accept loop */
    while (EXIT_SIGNAL == false) {
        /* accept a connection on a socket */
        struct sockaddr_storage client_addr;
 
        fd_accept = accept(socket_fd, (struct sockaddr*)&client_addr, &client_num);
        
        if (ret == -1) {
            printf("accept() failed!\n");
            return(ret);
        }

        /* Log connected client information */
        char client_address[INET_ADDRSTRLEN];
        get_client_info((struct sockaddr*)&client_addr, client_num, client_address);
   
        // allocate thread data
        thread_data * p_thread_data = malloc(sizeof(thread_data));
        if (p_thread_data == NULL){
            printf("malloc() failed!\n");
            return(RETCODE_FAILURE);
        }
        p_thread_data->mutex = &mutex;
        p_thread_data->fd_accept = fd_accept;
        strncpy(p_thread_data->client_address, client_address, INET_ADDRSTRLEN);
        p_thread_data->fp = fp;
   
        // start new thread
        pthread_t thread;
        ret = pthread_create(&thread, NULL, (void*)thread_socket_receive, p_thread_data);
        if (ret != 0 ){
            printf("pthread_create() failed!\n");
            return(RETCODE_FAILURE);
        } 
   
        /* Insert thread information to the linked list */
        struct node * p_node = NULL;
        p_node = malloc(sizeof(struct node));
        p_node->thread = thread;
        p_node->thread_exit = &p_thread_data->thread_exit;
        TAILQ_INSERT_TAIL(&head, p_node, nodes);
        p_node = NULL;
      
        check_thread_exit(&head);
    }
    
    /* stop all thread */
    release_all_thread(&head);

    /* close file descriptor */
    fclose(fp);
    shutdown(socket_fd, 2);
    close(socket_fd);

    /* remove mutex */
    pthread_mutex_destroy(&mutex);

    /* remove temp file */
    if (remove(TEMP_PATH) == 0) {
        printf("removed successfully\n");
    }

    printf("Exit...\n");
    /* return 0 to indicate success */
    return RETCODE_SUCCESS;
}
