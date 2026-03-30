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

} // namespace utils
} // namespace hft