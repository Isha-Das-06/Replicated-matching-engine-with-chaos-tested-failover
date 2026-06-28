#pragma once

#include "MiniRaft.h"
#include <vector>
#include <memory>

namespace consensus {

class MockCluster {
public:
    MockCluster(int num_nodes);
    ~MockCluster();
    
    // Finds current leader
    MiniRaftNode* get_leader();
    
    // Simulates a leader crash
    void kill_leader();

    std::vector<MiniRaftNode*> peers;

private:
    std::vector<std::unique_ptr<me::MatchingEngine>> engines_;
    std::vector<std::unique_ptr<MiniRaftNode>> nodes_;
};

} // namespace consensus
