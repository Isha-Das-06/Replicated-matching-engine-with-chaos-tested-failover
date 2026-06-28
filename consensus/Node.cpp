#include "Node.h"
#include <cstring>
#include <iostream>

namespace consensus {

Node::Node(int id, me::MatchingEngine& engine)
    : id_(id), is_leader_(false), is_alive_(true), state_machine_(engine), log_index_(0) {}

bool Node::submit(const RaftOrderPayload& payload) {
    if (!is_alive_) return false;
    if (!is_leader_) return false;

    // Serialize payload
    auto buf = nuraft::buffer::alloc(sizeof(RaftOrderPayload));
    std::memcpy(buf->data_begin(), &payload, sizeof(RaftOrderPayload));

    // Commit to state machine
    state_machine_.commit(++log_index_, *buf);
    return true;
}

MockCluster::MockCluster(int num_nodes) {
    // We instantiate 1 MatchingEngine per node, simulating independent node state
    for (int i = 0; i < num_nodes; ++i) {
        // We leak the engine memory in this simple mock cluster to avoid dealing with object lifetimes 
        // in a non-smart-pointer setup, but it's fine for the mock
        auto engine = new me::MatchingEngine();
        nodes.push_back(std::make_unique<Node>(i, *engine));
    }
    
    // Node 0 is initially leader
    if (!nodes.empty()) {
        nodes[0]->set_leader(true);
    }
}

Node* MockCluster::get_leader() {
    for (auto& node : nodes) {
        if (node->is_alive() && node->is_leader()) {
            return node.get();
        }
    }
    return nullptr;
}

void MockCluster::kill_leader() {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i]->is_leader() && nodes[i]->is_alive()) {
            nodes[i]->kill();
            nodes[i]->set_leader(false);
            
            // Elect new leader (just pick the next alive one)
            for (size_t j = 1; j < nodes.size(); ++j) {
                size_t next_idx = (i + j) % nodes.size();
                if (nodes[next_idx]->is_alive()) {
                    nodes[next_idx]->set_leader(true);
                    std::cout << "Node " << nodes[next_idx]->get_id() << " elected as new leader.\n";
                    break;
                }
            }
            break;
        }
    }
}

} // namespace consensus
