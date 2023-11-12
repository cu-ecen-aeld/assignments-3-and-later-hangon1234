#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

void error_checking()
{
    const int errno_local = errno;
    int size = snprintf(NULL, 0, "File open failed, message: %s\n", strerror(errno_local));
    char * buffer = malloc(size + 1);
    sprintf(buffer, "File open failed, message: %s\n", strerror(errno_local));
    syslog(LOG_USER, "%s\n", buffer);
}

int main(int argc, char * argv[])
{
    // Variable to check error
    int ret = 0;

    if (argc != 3) {
        printf("Check usage: ./writer {output_path} {text_string}\n");
        return(1);
    } 

    // Open connection to Syslog
    openlog("[writer]", LOG_PID|LOG_NDELAY, LOG_USER);

    // Get command line argument
    const char * text_string = argv[2];
    const char * output_path = argv[1];

    // Write a message 
    syslog(LOG_DEBUG, "Writing %s to %s\n", text_string, output_path);

    // Open file
    FILE * file = fopen(output_path, "w");

    // Check any unexpected error exists
    if (file == NULL) {
        error_checking();
    }

    // Write to file
    ret = fprintf(file, text_string, NULL);
    
    // Check any error
    // fprint negative return indicates error
    if (ret < 0)
    {
        error_checking();
    }

    return(0);
}	
