#pragma once

#include <vector>
#include "LobsterParser.h"
#include "../matching_engine/MatchingEngine.h"

namespace client {

class ReplayClient {
public:
    ReplayClient(me::MatchingEngine& engine);
    ~ReplayClient() = default;

    // Replays a list of messages into the engine and measures time
    // Prints out latency p99 and throughput (orders/sec)
    void replay(const std::vector<LobsterMessage>& messages);

private:
    me::MatchingEngine& engine_;
};

} // namespace client
