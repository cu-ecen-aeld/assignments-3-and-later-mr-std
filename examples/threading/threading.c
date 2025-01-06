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
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    usleep((thread_func_args->obtain_mutex_time)*1000);

    int retval = pthread_mutex_lock(thread_func_args->data_mutex);
    if(retval != 0)
    {
        printf("pthread_mutex_lock failed with %d\n", retval);
        thread_func_args->thread_complete_success = false;
    }

    usleep((thread_func_args->release_mutex_time)*1000);

    retval = pthread_mutex_unlock(thread_func_args->data_mutex);
    if(retval != 0)
    {
        printf("pthread_mutex_unlock failed with %d\n", retval);
        thread_func_args->thread_complete_success = false;
    }

    thread_func_args->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data *pData_struct = (struct thread_data *)malloc(sizeof(struct thread_data));
    pData_struct->data_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    if(pData_struct != NULL)
    {
        pData_struct->data_mutex = mutex;
        pData_struct->obtain_mutex_time = wait_to_obtain_ms;
        pData_struct->release_mutex_time = wait_to_release_ms;
        pData_struct->thread_complete_success = false;

        int retval = pthread_create(thread, NULL, threadfunc, (void *)pData_struct);
        if(retval != 0)
        {
            printf("Failed to creating the thread, error was %d\n", retval);
        }
        else
        {
            return true;
        }
    }

    return false;
}

