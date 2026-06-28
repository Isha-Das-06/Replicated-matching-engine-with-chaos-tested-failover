#pragma once

#include "Types.h"
#include <map>
#include <cstdint>
#include <vector>

namespace me {

// Represents a price level in the order book.
// It maintains a doubly linked list of orders for O(1) queue operations (like cancel).
class PriceLevel {
public:
    PriceLevel(uint64_t price) : price_(price), head_(nullptr), tail_(nullptr) {}

    void append(Order* order) {
        order->next = nullptr;
        order->prev = tail_;
        if (tail_) {
            tail_->next = order;
        } else {
            head_ = order;
        }
        tail_ = order;
    }

    void remove(Order* order) {
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head_ = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail_ = order->prev;
        }
    }

    Order* head() const { return head_; }
    bool empty() const { return head_ == nullptr; }

private:
    uint64_t price_;
    Order* head_;
    Order* tail_;
};

// Represents a match execution
struct Execution {
    uint64_t buy_id;
    uint64_t sell_id;
    uint64_t price;
    uint64_t quantity;
};

class OrderBook {
public:
    OrderBook();
    ~OrderBook() = default;

    // Process a new limit order, returns executions if matched
    std::vector<Execution> process_limit_order(Order* order);
    
    // Process a new market order, returns executions if matched
    std::vector<Execution> process_market_order(Order* order);

    void cancel_order(Order* order);

    // Get best bid and ask
    uint64_t best_bid() const;
    uint64_t best_ask() const;

private:
    std::map<uint64_t, PriceLevel, std::greater<uint64_t>> bids_; // descending
    std::map<uint64_t, PriceLevel, std::less<uint64_t>> asks_;    // ascending
};

} // namespace me
