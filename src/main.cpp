#include <iostream>
#include <thread>
#include <chrono>
#include "concurrency/spsc_queue.hpp"
#include "engine/order.hpp"
#include "utils/thread_utils.hpp"

using namespace hft;

// Simulated Network Gateway (The Producer)
void network_gateway_loop(SPSCQueue<Order, 1024>& ring_buffer) {
    uint64_t next_order_id = 1;
    
    std::cout << "[Gateway] Listening for network traffic...\n";

    while (true) {
        // 1. Simulate receiving an order from the network
        Order new_order(next_order_id++, 50000, 10, Side::BUY);

        // 2. Push to the lock-free queue (Spinlock if full)
        while (!ring_buffer.push(new_order)) {
            // In a real engine, we might yield or pause here, 
            // but for HFT we just aggressively spin-wait.
        }

        // 3. Sleep for a millisecond just so we don't blow up your terminal 
        // while you try to watch htop
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    std::cout << "=== Starting HFT Matching Engine ===\n";

    // Initialize our lock-free message bus
    SPSCQueue<Order, 1024> engine_queue;

    // 1. Spawn the Gateway (Producer) thread
    std::thread gateway_thread(network_gateway_loop, std::ref(engine_queue));

    // 2. PIN IT TO CORE 1
    // (Note: Cores are 0-indexed. Core 0 is your OS core. Core 1 is the 2nd core).
    if (utils::pin_thread_to_core(gateway_thread, 1)) {
        std::cout << "[System] Gateway thread successfully pinned to CPU Core 1.\n";
    }

    // 3. Dummy Consumer to drain the queue so it doesn't instantly fill up
    std::thread dummy_engine_thread([&]() {
        Order incoming;
        while (true) {
            if (engine_queue.pop(incoming)) {
                // (In Issue #10, we will hook this up to your OrderBook!)
            }
        }
    });

    // We also want to pin the matching engine to Core 2 so they don't fight!
    if (utils::pin_thread_to_core(dummy_engine_thread, 2)) {
        std::cout << "[System] Matching Engine thread successfully pinned to CPU Core 2.\n";
    }

    // Wait forever
    gateway_thread.join();
    dummy_engine_thread.join();

    return 0;
}