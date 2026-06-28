#pragma once

#include <cstdint>
#include <string>

namespace me {

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};

struct Order {
    uint64_t id;
    uint64_t client_id;
    uint64_t client_seq; // For idempotency
    Side side;
    OrderType type;
    uint64_t price; // Price in cents or ticks
    uint64_t quantity;
    uint64_t sequence_number; // Abstract monotonic sequence for deterministic tie-breaking

    Order* next; // Intrusive linked list pointer for the queue
    Order* prev; // Intrusive linked list pointer for the queue
};

} // namespace me
