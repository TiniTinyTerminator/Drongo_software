#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <thread>
#include <system_error>
#include <iostream>

#ifndef LINUX_SCHEDULING_H
#define LINUX_SCHEDULING_H

// Set the priority of a thread on a linux system
void set_realtime_priority()
{
    pthread_t this_thread = pthread_self();
    struct sched_param params;
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);

    // Attempt to set thread real-time priority to the SCHED_FIFO policy
    if (pthread_setschedparam(this_thread, SCHED_FIFO, &params) != 0)
        // Throw a system_error with the error code from errno
        throw std::system_error(errno, std::system_category(), "Failed to set real-time priority");

    // Now verify the change in thread priority
    int policy = 0;

    if (pthread_getschedparam(this_thread, &policy, &params) != 0)
        throw std::system_error(errno, std::system_category(), "Failed to get thread scheduling params");

    if (policy != SCHED_FIFO)
        throw std::runtime_error("Scheduling is not SCHED_FIFO");

    if (params.sched_priority != sched_get_priority_max(SCHED_FIFO))
        throw std::runtime_error("Priority is not set to max for SCHED_FIFO");
}

// Function that sets the thread affinity to a single core
void set_thread_affinity(int core_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
        // Throw a system_error with the error code from errno
        throw std::system_error(errno, std::system_category(), "Failed to set core isolation priority");
}

pthread_t get_pid(void)
{
    return pthread_self();
}

#endif