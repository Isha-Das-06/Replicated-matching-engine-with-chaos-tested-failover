#include "MatchingEngine.h"

namespace me {

MatchingEngine::MatchingEngine(size_t pool_capacity)
    : order_pool_(pool_capacity), current_sequence_(0) {}

std::vector<Execution> MatchingEngine::submit_order(
    uint64_t client_id,
    uint64_t client_seq,
    Side side,
    OrderType type,
    uint64_t price,
    uint64_t quantity
) {
    Order* order = order_pool_.allocate();
    order->id = ++current_sequence_;
    order->client_id = client_id;
    order->client_seq = client_seq;
    order->side = side;
    order->type = type;
    order->price = price;
    order->quantity = quantity;
    order->sequence_number = order->id;
    order->next = nullptr;
    order->prev = nullptr;

    std::vector<Execution> executions;

    if (type == OrderType::LIMIT) {
        executions = order_book_.process_limit_order(order);
        if (order->quantity > 0) {
            // It remains in the book, track it for potential cancellations
            active_orders_[order->id] = order;
        } else {
            // Fully filled immediately, deallocate
            order_pool_.deallocate(order);
        }
    } else if (type == OrderType::MARKET) {
        executions = order_book_.process_market_order(order);
        // Market orders don't rest in the book
        order_pool_.deallocate(order);
    }

    // Process active_orders cleanup for any resting orders that just got fully filled
    // In a real optimized engine, we'd integrate this tightly with process_limit_order
    for (const auto& exec : executions) {
        if (exec.buy_id != order->id) {
            auto it = active_orders_.find(exec.buy_id);
            if (it != active_orders_.end() && it->second->quantity == 0) {
                order_pool_.deallocate(it->second);
                active_orders_.erase(it);
            }
        }
        if (exec.sell_id != order->id) {
            auto it = active_orders_.find(exec.sell_id);
            if (it != active_orders_.end() && it->second->quantity == 0) {
                order_pool_.deallocate(it->second);
                active_orders_.erase(it);
            }
        }
    }

    return executions;
}

bool MatchingEngine::cancel_order(uint64_t order_id) {
    auto it = active_orders_.find(order_id);
    if (it != active_orders_.end()) {
        Order* order = it->second;
        order_book_.cancel_order(order);
        order_pool_.deallocate(order);
        active_orders_.erase(it);
        return true;
    }
    return false;
}

} // namespace me
