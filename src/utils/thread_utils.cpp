#include "utils/thread_utils.hpp"
#include <pthread.h> // Linux POSIX threads
#include <sched.h>   // Linux scheduler
#include <iostream>

namespace hft {
namespace utils {

    bool pin_thread_to_core(std::thread& t, int core_id) {
        // Create a CPU bitmask
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);          // Clear the mask
        CPU_SET(core_id, &cpuset);  // Set the specific core bit

        // Extract the raw Linux pthread handle from the C++ std::thread
        pthread_t raw_thread = t.native_handle();

        // Ask the Linux Kernel to pin it
        int rc = pthread_setaffinity_np(raw_thread, sizeof(cpu_set_t), &cpuset);

        if (rc != 0) {
            std::cerr << "[ERROR] Failed to pin thread to core " << core_id 
                      << ". OS returned code: " << rc << "\n";
            return false;
        }

        return true;
    }

    bool set_realtime_priority(std::thread& t) {
        sched_param sch;
        int policy;
        pthread_t raw_thread = t.native_handle();

        // Get the current scheduling parameters
        pthread_getschedparam(raw_thread, &policy, &sch);
        
        // Set to maximum possible priority for FIFO real-time tasks
        sch.sched_priority = sched_get_priority_max(SCHED_FIFO);

        // Apply the Real-Time policy to the Linux Kernel
        int rc = pthread_setschedparam(raw_thread, SCHED_FIFO, &sch);
        if (rc != 0) {
            std::cerr << "[WARNING] Failed to set SCHED_FIFO real-time priority. "
                      << "(Note: This usually requires running the executable with 'sudo'). "
                      << "OS returned code: " << rc << "\n";
            return false;
        }
        return true;
    }

} // namespace utils
} // namespace hft