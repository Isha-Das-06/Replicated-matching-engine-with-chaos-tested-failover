#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <functional>
#include "../matching_engine/MatchingEngine.h"

namespace consensus {

enum class RaftState {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

enum class RpcType {
    APPEND_ENTRIES,
    REQUEST_VOTE,
    APPEND_ENTRIES_REPLY,
    REQUEST_VOTE_REPLY,
    CLIENT_REQUEST
};

struct RpcMessage {
    RpcType type;
    int from_id;
    int to_id;
    int term;
    
    // For RequestVote
    int candidate_id;
    bool vote_granted;

    // For AppendEntries
    uint64_t prev_log_index;
    bool success;
    
    // For Client Requests
    uint64_t client_id;
    uint64_t client_seq;
    me::Side side;
    me::OrderType order_type;
    uint64_t price;
    uint64_t quantity;
};

class ThreadSafeQueue {
public:
    void push(const RpcMessage& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cond_.notify_one();
    }

    bool pop_with_timeout(RpcMessage& msg, int timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cond_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]() { return !queue_.empty(); })) {
            msg = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }

private:
    std::queue<RpcMessage> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class MiniRaftNode {
public:
    MiniRaftNode(int id, me::MatchingEngine& engine);
    ~MiniRaftNode();

    void set_peers(const std::vector<MiniRaftNode*>& peers) { peers_ = peers; }

    void start();
    void stop();
    void kill(); // Simulates ungraceful death
    void send_rpc(const RpcMessage& msg);
    
    // For the client
    bool submit(const RpcMessage& request);

    bool is_leader() const { return state_ == RaftState::LEADER; }
    int get_id() const { return id_; }
    bool is_alive() const { return alive_.load(); }

private:
    void loop();
    void reset_election_timer();
    void become_follower(int term);
    void become_candidate();
    void become_leader();
    
    void handle_request_vote(const RpcMessage& msg);
    void handle_request_vote_reply(const RpcMessage& msg);
    void handle_append_entries(const RpcMessage& msg);
    void handle_append_entries_reply(const RpcMessage& msg);
    void handle_client_request(const RpcMessage& msg);

    int id_;
    std::vector<MiniRaftNode*> peers_;
    me::MatchingEngine& engine_;

    std::atomic<bool> running_;
    std::atomic<bool> alive_;
    std::thread thread_;
    ThreadSafeQueue queue_;

    RaftState state_;
    int current_term_;
    int voted_for_;
    int votes_received_;

    uint64_t commit_index_;
    std::chrono::time_point<std::chrono::steady_clock> last_heartbeat_;
    int election_timeout_ms_;
};

} // namespace consensus
