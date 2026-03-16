#include "engine/order.hpp"
#include <algorithm>

namespace hft {
    void OrderBook::match_buy(Order& order){
        auto it = asks.begin();
        // Outer loop: Traverse the map using the safe iterator pattern
        while (it != asks.end() && order.quantity > 0 && it->first <= order.price){
            auto& order_list = it->second;

            // Inner loop: Strict FIFO Queue handling
            while (!order_list.empty() && order.quantity > 0) {
                Order& top = order_list.front();
                u32 traded = std::min(order.quantity, top.quantity);

                order.quantity -= traded;
                top.quantity -= traded;

                if (top.quantity == 0) {
                    order_index.erase(top.id); // Keep index in sync
                    order_list.pop_front();    // Safe, non-iterator deletion
                }
            }  
            
            // Safe map erasure
            if (order_list.empty()) it = asks.erase(it);
            else ++it;          
        }

        // If not fully filled, add to resting bids and track in index
        if (order.quantity > 0){
            auto& lst = bids[order.price];
            lst.push_back(order);
            order_index[order.id] = std::prev(lst.end());
        }  
    }

    void OrderBook::match_sell(Order& order) {
        auto it = bids.begin();
        // Outer loop: Traverse the map using the safe iterator pattern
        while (it != bids.end() && order.quantity > 0 && it->first >= order.price) {
            auto& order_list = it->second;

            // Inner loop: Strict FIFO Queue handling
            while (!order_list.empty() && order.quantity > 0) {
                Order& top = order_list.front();
                u32 traded = std::min(order.quantity, top.quantity);

                order.quantity -= traded;
                top.quantity -= traded;

                if (top.quantity == 0) {
                    order_index.erase(top.id); // Keep index in sync
                    order_list.pop_front();    // Safe, non-iterator deletion
                }
            }

            // Safe map erasure (From Second Implementation)
            if (order_list.empty()) it = bids.erase(it);
            else ++it;
            
        }

        // If not fully filled, add to resting asks and track in index
        if (order.quantity > 0) {
            auto& lst = asks[order.price];
            lst.push_back(order);
            order_index[order.id] = std::prev(lst.end());
        }
    }

    void OrderBook::add_order(const Order& in_order) {
        if (in_order.quantity == 0) return;
        if (order_index.count(in_order.id)) return; // Defensive check to prevent duplicate order IDs corrupting the index
        // Explicit mutable copy
        Order order = in_order;
        
        if (order.side == Side::BUY) match_buy(order);
        else match_sell(order);
    }

    void OrderBook::cancel_order(u64 order_id) {
        // Find the order in the lookup table
        // If not found, nothing to cancel
        auto it = order_index.find(order_id);

        // Iterator pointing to the order inside the list
        if (it == order_index.end()) return;
        auto list_it = it->second;

        // We need price + side to locate correct book
        u32 price = list_it->price;
        Side side = list_it->side;

        if (side == Side::BUY) {
            // Find price level in bid book safely
            auto map_it = bids.find(price);

            if (map_it != bids.end()) {
                map_it->second.erase(list_it);
                if (map_it->second.empty()) bids.erase(map_it);
            }
        } 
        
        else {
            // Same logic for ask side
            auto map_it = asks.find(price);

            if (map_it != asks.end()) {
                map_it->second.erase(list_it);
                if (map_it->second.empty()) asks.erase(map_it);
            }
        }
        // Finally, remove the tracking iterator from the index to prevent dangling pointers
        order_index.erase(it);
    }

}