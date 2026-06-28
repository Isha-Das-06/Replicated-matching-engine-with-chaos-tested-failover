#include "MiniRaft.h"
#include <random>
#include <iostream>

namespace consensus {

MiniRaftNode::MiniRaftNode(int id, me::MatchingEngine& engine)
    : id_(id), engine_(engine), running_(false), alive_(true),
      state_(RaftState::FOLLOWER), current_term_(0), voted_for_(-1), votes_received_(0),
      commit_index_(0) {
    reset_election_timer();
}

MiniRaftNode::~MiniRaftNode() {
    stop();
}

void MiniRaftNode::start() {
    running_ = true;
    alive_ = true;
    thread_ = std::thread(&MiniRaftNode::loop, this);
}

void MiniRaftNode::stop() {
    running_ = false;
    // Push dummy message to wake up cond var
    queue_.push(RpcMessage{RpcType::APPEND_ENTRIES, -1, id_, 0});
    if (thread_.joinable()) {
        thread_.join();
    }
}

void MiniRaftNode::kill() {
    alive_ = false;
    std::cout << "Node " << id_ << " KILLED.\n";
}

void MiniRaftNode::send_rpc(const RpcMessage& msg) {
    if (!alive_) return;
    queue_.push(msg);
}

bool MiniRaftNode::submit(const RpcMessage& request) {
    if (!alive_ || state_ != RaftState::LEADER) return false;
    queue_.push(request);
    return true;
}

void MiniRaftNode::reset_election_timer() {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count() + id_ * 1000;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(150, 300);
    election_timeout_ms_ = dis(gen);
    last_heartbeat_ = std::chrono::steady_clock::now();
}

void MiniRaftNode::become_follower(int term) {
    state_ = RaftState::FOLLOWER;
    current_term_ = term;
    voted_for_ = -1;
    reset_election_timer();
}

void MiniRaftNode::become_candidate() {
    state_ = RaftState::CANDIDATE;
    current_term_++;
    voted_for_ = id_;
    votes_received_ = 1;
    reset_election_timer();

    RpcMessage req;
    req.type = RpcType::REQUEST_VOTE;
    req.from_id = id_;
    req.term = current_term_;
    req.candidate_id = id_;

    for (auto* peer : peers_) {
        if (peer->get_id() != id_) {
            req.to_id = peer->get_id();
            peer->send_rpc(req);
        }
    }
}

void MiniRaftNode::become_leader() {
    state_ = RaftState::LEADER;
    std::cout << "Node " << id_ << " became LEADER for term " << current_term_ << "\n";
    // Send initial heartbeats immediately
    RpcMessage hb;
    hb.type = RpcType::APPEND_ENTRIES;
    hb.from_id = id_;
    hb.term = current_term_;
    for (auto* peer : peers_) {
        if (peer->get_id() != id_) {
            hb.to_id = peer->get_id();
            peer->send_rpc(hb);
        }
    }
}

void MiniRaftNode::loop() {
    while (running_) {
        if (!alive_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        RpcMessage msg;
        // Calculate remaining timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_heartbeat_).count();
        int wait_time = election_timeout_ms_ - elapsed;

        if (state_ == RaftState::LEADER) {
            wait_time = 50; // Heartbeat interval
        }

        if (wait_time <= 0) {
            if (state_ != RaftState::LEADER) {
                become_candidate();
            } else {
                // Send heartbeats
                RpcMessage hb;
                hb.type = RpcType::APPEND_ENTRIES;
                hb.from_id = id_;
                hb.term = current_term_;
                for (auto* peer : peers_) {
                    if (peer->get_id() != id_) {
                        hb.to_id = peer->get_id();
                        peer->send_rpc(hb);
                    }
                }
                last_heartbeat_ = std::chrono::steady_clock::now();
            }
            continue;
        }

        if (queue_.pop_with_timeout(msg, wait_time)) {
            if (!alive_) continue;

            if (msg.term > current_term_ && msg.type != RpcType::CLIENT_REQUEST) {
                become_follower(msg.term);
            }

            switch (msg.type) {
                case RpcType::REQUEST_VOTE: handle_request_vote(msg); break;
                case RpcType::REQUEST_VOTE_REPLY: handle_request_vote_reply(msg); break;
                case RpcType::APPEND_ENTRIES: handle_append_entries(msg); break;
                case RpcType::APPEND_ENTRIES_REPLY: handle_append_entries_reply(msg); break;
                case RpcType::CLIENT_REQUEST: handle_client_request(msg); break;
            }
        }
    }
}

void MiniRaftNode::handle_request_vote(const RpcMessage& msg) {
    RpcMessage reply;
    reply.type = RpcType::REQUEST_VOTE_REPLY;
    reply.from_id = id_;
    reply.to_id = msg.from_id;
    reply.term = current_term_;
    reply.vote_granted = false;

    if (msg.term >= current_term_ && (voted_for_ == -1 || voted_for_ == msg.candidate_id)) {
        reply.vote_granted = true;
        voted_for_ = msg.candidate_id;
        reset_election_timer();
    }

    for (auto* peer : peers_) {
        if (peer->get_id() == msg.from_id) {
            peer->send_rpc(reply);
            break;
        }
    }
}

void MiniRaftNode::handle_request_vote_reply(const RpcMessage& msg) {
    if (state_ == RaftState::CANDIDATE && msg.term == current_term_ && msg.vote_granted) {
        votes_received_++;
        if (votes_received_ > peers_.size() / 2) {
            become_leader();
        }
    }
}

void MiniRaftNode::handle_append_entries(const RpcMessage& msg) {
    reset_election_timer();
    
    RpcMessage reply;
    reply.type = RpcType::APPEND_ENTRIES_REPLY;
    reply.from_id = id_;
    reply.to_id = msg.from_id;
    reply.term = current_term_;
    reply.success = true;

    for (auto* peer : peers_) {
        if (peer->get_id() == msg.from_id) {
            peer->send_rpc(reply);
            break;
        }
    }
}

void MiniRaftNode::handle_append_entries_reply(const RpcMessage& msg) {
    // In a real Raft, we update matchIndex here and advance commitIndex.
    // For this mock, we assume success means quorum, but since we just want the election timing,
    // we process client requests immediately upon receipt for benchmark simplicity.
}

void MiniRaftNode::handle_client_request(const RpcMessage& msg) {
    if (state_ == RaftState::LEADER) {
        // Real Raft would append to log and wait for quorum APPEND_ENTRIES_REPLY.
        // For our test, we just execute on the engine to show it handles it,
        // since the goal is testing election timeout recovery.
        engine_.submit_order(msg.client_id, msg.client_seq, msg.side, msg.order_type, msg.price, msg.quantity);
        commit_index_++;
    }
}

} // namespace consensus
