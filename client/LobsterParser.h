#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../matching_engine/Types.h"

namespace client {

struct LobsterMessage {
    double time;
    int event_type;
    uint64_t order_id;
    uint64_t size;
    uint64_t price;
    int direction; // 1 for Buy, -1 for Sell
};

class LobsterParser {
public:
    LobsterParser(const std::string& filepath);
    ~LobsterParser() = default;

    // Parses the file and returns a list of actionable orders
    // Maps Lobster Event Types to our internal matching engine operations
    // e.g. Event 1 -> Limit Order
    // Event 2/3 -> Cancel (Wait, our ME currently only takes Limit/Market)
    // We will parse them into our me::Order format or a format the client can send
    std::vector<LobsterMessage> parse();

private:
    std::string filepath_;
};

} // namespace client
