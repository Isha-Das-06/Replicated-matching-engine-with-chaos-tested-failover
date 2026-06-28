#pragma once

#include "OrderBook.h"
#include "ObjectPool.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace me {

class MatchingEngine {
public:
    MatchingEngine(size_t pool_capacity = 1000000);
    ~MatchingEngine() = default;

    // Submits a new order and returns a list of executions
    std::vector<Execution> submit_order(
        uint64_t client_id,
        uint64_t client_seq,
        Side side,
        OrderType type,
        uint64_t price,
        uint64_t quantity
    );

    // Cancels an existing order by ID
    bool cancel_order(uint64_t order_id);

    uint64_t get_best_bid() const { return order_book_.best_bid(); }
    uint64_t get_best_ask() const { return order_book_.best_ask(); }

private:
    OrderBook order_book_;
    ObjectPool<Order> order_pool_;
    
    // Quick lookup for cancels
    std::unordered_map<uint64_t, Order*> active_orders_;
    
    // Abstract monotonic sequence for tie-breaking and idempotency
    uint64_t current_sequence_; 
    
    // Helper to cleanup filled orders from memory
    void cleanup_filled_orders(const std::vector<Execution>& executions);
};

} // namespace me
