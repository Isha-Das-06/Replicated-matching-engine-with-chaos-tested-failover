#include "Node.h"
#include "../client/LobsterParser.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

using namespace consensus;
using namespace client;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_lobster_csv>\n";
        return 1;
    }

    std::string filepath = argv[1];
    
    std::cout << "Parsing LOBSTER file for Chaos Test...\n";
    LobsterParser parser(filepath);
    auto messages = parser.parse();
    std::cout << "Parsed " << messages.size() << " messages.\n";
    
    MockCluster cluster(3);
    std::cout << "Cluster initialized with 3 nodes.\n";

    uint64_t client_seq = 0;
    size_t msgs_processed = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    long long failover_start_ns = 0;
    long long failover_end_ns = 0;

    for (const auto& msg : messages) {
        if (msg.event_type != 1) continue;
        
        // At exactly halfway, KILL the leader
        if (msgs_processed == messages.size() / 2) {
            std::cout << "Injecting failure: KILLING LEADER!\n";
            auto f_start = std::chrono::high_resolution_clock::now();
            cluster.kill_leader();
            auto f_end = std::chrono::high_resolution_clock::now();
            failover_start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(f_start.time_since_epoch()).count();
            failover_end_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(f_end.time_since_epoch()).count();
        }

        RaftOrderPayload payload;
        payload.client_id = 1;
        payload.client_seq = ++client_seq;
        payload.side = (msg.direction == 1) ? me::Side::BUY : me::Side::SELL;
        payload.type = me::OrderType::LIMIT;
        payload.price = msg.price;
        payload.quantity = msg.size;

        bool success = false;
        while (!success) {
            Node* leader = cluster.get_leader();
            if (leader) {
                success = leader->submit(payload);
            }
            if (!success) {
                // If submit fails, it means we hit a dead node or non-leader. 
                // Client retry loop.
            }
        }
        msgs_processed++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    
    double total_sec = total_ns / 1e9;
    double throughput = msgs_processed / total_sec;
    double failover_ms = (failover_end_ns - failover_start_ns) / 1e6;

    std::cout << "--- Chaos Test Results ---\n";
    std::cout << "Total Orders Processed: " << msgs_processed << "\n";
    std::cout << "Throughput (incl failover): " << throughput << " orders/sec\n";
    std::cout << "Failover Recovery Time: " << failover_ms << " ms\n";
    std::cout << "--------------------------\n";

    return 0;
}
