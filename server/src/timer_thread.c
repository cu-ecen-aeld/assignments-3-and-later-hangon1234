// this file will write timestamp in every 10 seconds
#include "aesdsocket.h"

extern bool EXIT_SIGNAL;

void timer_thread(union sigval sigval)
{
    /* get thread data set at timer_create */
    struct thread_data *td = (struct thread_data*) sigval.sival_ptr;

    /* Get a lock to prevent interleaving output from socket thread */
    pthread_mutex_lock(td->mutex);

    /* append timestamp to the output file */
    char buf[BUFLEN];
    buf[BUFLEN] = '\0';

    /* Get current timestamp */
    time_t current_time = time(NULL);
    int timestamp_length;
    timestamp_length = strftime(buf, BUFLEN, "timestamp:%a, %d %b %Y %T %z\n", localtime(&current_time));

    /* Write timestamp to the file */
    fwrite(buf, timestamp_length, 1, td->fp);

    printf("Debugging: timer_thread is working\n");
    /* Release lock */
    pthread_mutex_unlock(td->mutex);
}
