/**
 * @file linux_scheduling.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-11-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef LINUX_SCHEDULING_H
#define LINUX_SCHEDULING_H

#include <pthread.h>
#include <stdexcept>
#include <system_error>

void set_thread_priority(int prio = 0, int scheduler = SCHED_FIFO)
{
    pthread_t this_thread = pthread_self();
    struct sched_param params;

    if (scheduler == SCHED_OTHER) {
        // For SCHED_OTHER, use nice value to change priority
        if (nice(prio) == -1 && errno != 0) {
            throw std::system_error(errno, std::system_category(), "Failed to set nice value");
        }
    } else {
        // Set priority within the valid range for real-time policies
        params.sched_priority = std::min(prio, sched_get_priority_max(scheduler));

        // Set thread real-time priority
        if (pthread_setschedparam(this_thread, scheduler, &params) != 0) {
            throw std::system_error(errno, std::system_category(), "Failed to set real-time priority");
        }

        // Verify the change in thread priority
        int policy = 0;
        if (pthread_getschedparam(this_thread, &policy, &params) != 0) {
            throw std::system_error(errno, std::system_category(), "Failed to get thread scheduling params");
        }

        if (policy != scheduler) {
            throw std::runtime_error("Scheduling policy did not match the requested policy");
        }

        if (params.sched_priority != prio) {
            throw std::runtime_error("Priority is not set as requested");
        }
    }
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