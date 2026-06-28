#include "OrderBook.h"
#include <algorithm>

namespace me {

OrderBook::OrderBook() {}

std::vector<Execution> OrderBook::process_limit_order(Order* order) {
    std::vector<Execution> executions;
    uint64_t qty_to_fill = order->quantity;

    if (order->side == Side::BUY) {
        auto it = asks_.begin();
        while (it != asks_.end() && it->first <= order->price && qty_to_fill > 0) {
            PriceLevel& level = it->second;
            Order* current = level.head();
            while (current && qty_to_fill > 0) {
                uint64_t exec_qty = std::min(qty_to_fill, current->quantity);
                executions.push_back({order->id, current->id, it->first, exec_qty});
                
                qty_to_fill -= exec_qty;
                current->quantity -= exec_qty;
                
                Order* next = current->next;
                if (current->quantity == 0) {
                    level.remove(current);
                    // In a real implementation we would deallocate 'current' from pool here
                    // or return a list of pointers to deallocate.
                }
                current = next;
            }
            if (level.empty()) {
                it = asks_.erase(it);
            } else {
                ++it;
            }
        }
        
        if (qty_to_fill > 0) {
            order->quantity = qty_to_fill;
            auto [level_it, inserted] = bids_.try_emplace(order->price, order->price);
            level_it->second.append(order);
        }
    } else {
        auto it = bids_.begin();
        while (it != bids_.end() && it->first >= order->price && qty_to_fill > 0) {
            PriceLevel& level = it->second;
            Order* current = level.head();
            while (current && qty_to_fill > 0) {
                uint64_t exec_qty = std::min(qty_to_fill, current->quantity);
                executions.push_back({current->id, order->id, it->first, exec_qty});
                
                qty_to_fill -= exec_qty;
                current->quantity -= exec_qty;
                
                Order* next = current->next;
                if (current->quantity == 0) {
                    level.remove(current);
                    // Deallocate current
                }
                current = next;
            }
            if (level.empty()) {
                it = bids_.erase(it);
            } else {
                ++it;
            }
        }
        
        if (qty_to_fill > 0) {
            order->quantity = qty_to_fill;
            auto [level_it, inserted] = asks_.try_emplace(order->price, order->price);
            level_it->second.append(order);
        }
    }
    
    return executions;
}

std::vector<Execution> OrderBook::process_market_order(Order* order) {
    std::vector<Execution> executions;
    uint64_t qty_to_fill = order->quantity;

    if (order->side == Side::BUY) {
        auto it = asks_.begin();
        while (it != asks_.end() && qty_to_fill > 0) {
            PriceLevel& level = it->second;
            Order* current = level.head();
            while (current && qty_to_fill > 0) {
                uint64_t exec_qty = std::min(qty_to_fill, current->quantity);
                executions.push_back({order->id, current->id, it->first, exec_qty});
                
                qty_to_fill -= exec_qty;
                current->quantity -= exec_qty;
                
                Order* next = current->next;
                if (current->quantity == 0) {
                    level.remove(current);
                }
                current = next;
            }
            if (level.empty()) {
                it = asks_.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        auto it = bids_.begin();
        while (it != bids_.end() && qty_to_fill > 0) {
            PriceLevel& level = it->second;
            Order* current = level.head();
            while (current && qty_to_fill > 0) {
                uint64_t exec_qty = std::min(qty_to_fill, current->quantity);
                executions.push_back({current->id, order->id, it->first, exec_qty});
                
                qty_to_fill -= exec_qty;
                current->quantity -= exec_qty;
                
                Order* next = current->next;
                if (current->quantity == 0) {
                    level.remove(current);
                }
                current = next;
            }
            if (level.empty()) {
                it = bids_.erase(it);
            } else {
                ++it;
            }
        }
    }
    return executions;
}

void OrderBook::cancel_order(Order* order) {
    if (order->side == Side::BUY) {
        auto it = bids_.find(order->price);
        if (it != bids_.end()) {
            it->second.remove(order);
            if (it->second.empty()) {
                bids_.erase(it);
            }
        }
    } else {
        auto it = asks_.find(order->price);
        if (it != asks_.end()) {
            it->second.remove(order);
            if (it->second.empty()) {
                asks_.erase(it);
            }
        }
    }
}

uint64_t OrderBook::best_bid() const {
    if (bids_.empty()) return 0;
    return bids_.begin()->first;
}

uint64_t OrderBook::best_ask() const {
    if (asks_.empty()) return 0;
    return asks_.begin()->first;
}

} // namespace me
