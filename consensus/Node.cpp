#include "Node.h"
#include <iostream>

namespace consensus {

MockCluster::MockCluster(int num_nodes) {
    for (int i = 0; i < num_nodes; ++i) {
        engines_.push_back(std::make_unique<me::MatchingEngine>());
    }

    for (int i = 0; i < num_nodes; ++i) {
        nodes_.push_back(std::make_unique<MiniRaftNode>(i, *engines_[i]));
    }

    for (int i = 0; i < num_nodes; ++i) {
        peers.push_back(nodes_[i].get());
    }
    
    for (int i = 0; i < num_nodes; ++i) {
        nodes_[i]->set_peers(peers);
    }

    // Start all nodes
    for (auto& node : nodes_) {
        node->start();
    }
}

MockCluster::~MockCluster() {
    for (auto& node : nodes_) {
        node->stop();
    }
}

MiniRaftNode* MockCluster::get_leader() {
    for (auto& node : nodes_) {
        if (node->is_alive() && node->is_leader()) {
            return node.get();
        }
    }
    return nullptr;
}

void MockCluster::kill_leader() {
    for (auto& node : nodes_) {
        if (node->is_leader() && node->is_alive()) {
            node->kill();
            break;
        }
    }
}

} // namespace consensus
