#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data * p_thread_data = (struct thread_data*) thread_param;
    // wait
    usleep(1000*p_thread_data->sleep_ms);
    // obtain mutex
    pthread_mutex_lock(p_thread_data->mutex);
    // wait
    usleep(1000*p_thread_data->wait_ms);
    // release mutex
    pthread_mutex_unlock(p_thread_data->mutex);
    // indicate success
    p_thread_data->thread_complete_success = true;
    //return thread param pointer
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * See implementation details in threading.h file comment block
     */
    // Create thread_data
    struct thread_data * p_thread_data = malloc(sizeof(struct thread_data));
    // setup mutex
    p_thread_data->mutex = mutex;
    // setup wait arguments
    p_thread_data->wait_ms = wait_to_obtain_ms;
    // setup sleep ms
    p_thread_data->sleep_ms = wait_to_release_ms;

    // Create thread
    bool result = !pthread_create(thread, NULL, threadfunc, p_thread_data);

    // return result
    return result;
}

