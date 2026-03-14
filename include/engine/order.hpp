#pragma once

#include <cstdint>
#include <map>
#include <list>
#include <unordered_map>
#include <algorithm>

namespace hft {

    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    enum class Side {
        BUY, SELL
    };

    struct Order {
          u64 id;
          u32 price;        // price in ticks instead of floating point
          u32 quantity;
          Side side;

          Order(u64 id, u32 price, u32 quantity, Side side) 
            : id(id), price(price), quantity(quantity), side(side) {}
    };

    class OrderBook {

    // list to ensure FIFO ordering for same value of price
    // and ensure deletion from middle too if order cancelled
    private:
        std::map<u32, std::list<Order>, std::greater<u32>> bids;
        std::map<u32, std::list<Order>, std::less<u32>> asks;

        // lookup Table: maps Order ID -> location in the price list
        std::unordered_map<u64, std::list<Order>::iterator> order_index;

        // helper function : MATCH_BUY
        void match_buy(Order& order){
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

        // helper function : MATCH_SELL
        void match_sell(Order& order) {
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

    public:
        OrderBook() = default;
        ~OrderBook() = default;

        void add_order(const Order& in_order) {
            if (in_order.quantity == 0) return;
            if (order_index.count(in_order.id)) return; // Defensive check to prevent duplicate order IDs corrupting the index
            // Explicit mutable copy
            Order order = in_order;
            
            if (order.side == Side::BUY) match_buy(order);
            else match_sell(order);
        }

        void cancel_order(u64 order_id) {
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
        
        // Helper methods for GoogleTest validation
        bool has_bid(u32 price) const { return bids.find(price) != bids.end(); }
        bool has_ask(u32 price) const { return asks.find(price) != asks.end(); }
        bool is_tracked(u64 order_id) const { return order_index.find(order_id) != order_index.end(); }
    };
}
