#pragma once

#include "RaftStateMachine.h"
#include <vector>
#include <string>

namespace consensus {

// A mock Raft Node that demonstrates leader election, appending entries, and failover handling.
class Node {
public:
    Node(int id, me::MatchingEngine& engine);
    ~Node() = default;

    int get_id() const { return id_; }
    bool is_leader() const { return is_leader_; }
    void set_leader(bool leader) { is_leader_ = leader; }
    void kill() { is_alive_ = false; }
    void start() { is_alive_ = true; }
    bool is_alive() const { return is_alive_; }

    // Client calls submit to the cluster. If this node is not leader, it should return false
    // so the client can retry and discover the leader.
    bool submit(const RaftOrderPayload& payload);

private:
    int id_;
    bool is_leader_;
    bool is_alive_;
    RaftStateMachine state_machine_;
    uint64_t log_index_;
};

class MockCluster {
public:
    MockCluster(int num_nodes);
    
    // Finds current leader
    Node* get_leader();
    
    // Simulates a leader crash
    void kill_leader();

    std::vector<std::unique_ptr<Node>> nodes;
};

} // namespace consensus
