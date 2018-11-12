#ifndef UPDATE_EVENT
#define UPDATE_EVENT

#include <semaphore.h>

// wrapper for a semaphore
class Semaphore
{
public:
    virtual void signal() = 0;
    virtual void wait() = 0;
};

class LinuxSemaphore : public Semaphore
{
public:
    LinuxSemaphore()
    {
        sem_init(&semaphore, 0, 0);
    }

    virtual ~LinuxSemaphore()
    {
        sem_destroy(&semaphore);
    }

    void signal() override
    {
        sem_post(&semaphore);
    }

    void wait() override
    {
        sem_wait(&semaphore);
    }

private:
    sem_t semaphore;
};

#endif