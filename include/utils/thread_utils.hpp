#pragma once

#include <thread>

namespace hft {
namespace utils {

    /**
     * @brief Pins a standard C++ thread to a specific physical CPU core.
     * @param t The std::thread object to pin.
     * @param core_id The hardware core ID (0-indexed. e.g., 1 is the second core).
     * @return true if successful, false if the OS rejected it.
     */
    bool pin_thread_to_core(std::thread& t, int core_id);
    
    /**
     * @brief Elevates the thread to Real-Time (RT) scheduling policy (SCHED_FIFO).
     * This instructs the Linux kernel to NEVER preempt this thread for normal OS tasks.
     * @param t The std::thread object to elevate.
     * @return true if successful (usually requires sudo/root), false otherwise.
     */
    bool set_realtime_priority(std::thread& t);

} // namespace utils
} // namespace hft