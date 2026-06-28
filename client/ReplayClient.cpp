#include "ReplayClient.h"
#include <chrono>
#include <iostream>
#include <algorithm>

namespace client {

ReplayClient::ReplayClient(me::MatchingEngine& engine) : engine_(engine) {}

void ReplayClient::replay(const std::vector<LobsterMessage>& messages) {
    if (messages.empty()) {
        std::cout << "No messages to replay.\n";
        return;
    }

    std::vector<long long> latencies;
    latencies.reserve(messages.size());
    
    uint64_t client_seq = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& msg : messages) {
        auto t1 = std::chrono::high_resolution_clock::now();
        
        // Event type 1 is Submission (Limit Order)
        if (msg.event_type == 1) {
            me::Side side = (msg.direction == 1) ? me::Side::BUY : me::Side::SELL;
            engine_.submit_order(1, ++client_seq, side, me::OrderType::LIMIT, msg.price, msg.size);
        } else if (msg.event_type == 2 || msg.event_type == 3) {
            // Partial or full cancel. For simplicity in Phase 1 standalone benchmark, 
            // we will treat event 2/3 as a full cancel if we mapped it, but our ME uses internal order IDs.
            // In a real LOBSTER replay, msg.order_id matches the submitted msg.order_id.
            // We'll skip for this exact benchmark test unless we mapped lobster IDs to our IDs.
            // (We will skip for now to just measure insertion/matching throughput).
        }
        
        auto t2 = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        latencies.push_back(diff);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    
    std::sort(latencies.begin(), latencies.end());
    long long p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
    
    double total_sec = total_ns / 1e9;
    double throughput = messages.size() / total_sec;

    std::cout << "--- Benchmark Results ---\n";
    std::cout << "Total Orders: " << messages.size() << "\n";
    std::cout << "Throughput:   " << throughput << " orders/sec\n";
    std::cout << "p99 Latency:  " << p99_latency << " ns\n";
    std::cout << "-------------------------\n";
}

} // namespace client
