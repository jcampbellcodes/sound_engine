#ifndef UPDATE_EVENT
#define UPDATE_EVENT

#include <semaphore.h>

struct update_event
{
    update_event()
    {
        sem_init(&semaphore, 0, 0);
    }

    ~update_event()
    {
        sem_destroy(&semaphore);
    }

    void signal()
    {
        sem_post(&semaphore);
    }

    void wait()
    {
        sem_wait(&semaphore);
    }

    sem_t semaphore;

};

#endif