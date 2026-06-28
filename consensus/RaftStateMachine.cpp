#include "RaftStateMachine.h"
#include <iostream>
#include <cstring>

namespace consensus {

RaftStateMachine::RaftStateMachine(me::MatchingEngine& engine)
    : engine_(engine), last_committed_idx_(0) {}

nuraft::ptr<nuraft::buffer> RaftStateMachine::commit(const uint64_t log_idx, nuraft::buffer& data) {
    std::lock_guard<std::mutex> lock(engine_mutex_);
    
    if (data.size() == sizeof(RaftOrderPayload)) {
        RaftOrderPayload payload;
        std::memcpy(&payload, data.data_begin(), sizeof(RaftOrderPayload));
        
        // Execute against the matching engine
        engine_.submit_order(
            payload.client_id,
            payload.client_seq,
            payload.side,
            payload.type,
            payload.price,
            payload.quantity
        );
    }
    
    last_committed_idx_ = log_idx;
    
    // Return a dummy buffer as result
    return nuraft::buffer::alloc(1);
}

nuraft::ptr<nuraft::buffer> RaftStateMachine::commit_config(const uint64_t log_idx, nuraft::ptr<nuraft::cluster_config>& new_conf) {
    last_committed_idx_ = log_idx;
    return nuraft::buffer::alloc(1);
}

void RaftStateMachine::rollback(const uint64_t log_idx, nuraft::buffer& data) {
    // In a fully robust system, you might need to revert state if log_idx < last_committed_idx_.
    // Raft generally won't commit then rollback, but we must implement the interface.
}

int RaftStateMachine::read_logical_snp_obj(nuraft::snapshot& s, void*& user_snp_ctx, unsigned long obj_id, nuraft::ptr<nuraft::buffer>& data_out, bool& is_last_obj) {
    is_last_obj = true;
    return 0;
}

void RaftStateMachine::save_logical_snp_obj(nuraft::snapshot& s, unsigned long& obj_id, nuraft::buffer& data, bool is_first_obj, bool is_last_obj) {
}

bool RaftStateMachine::apply_snapshot(nuraft::snapshot& s) {
    return true;
}

void RaftStateMachine::free_user_snp_ctx(void*& user_snp_ctx) {
}

nuraft::ptr<nuraft::snapshot> RaftStateMachine::last_snapshot() {
    return last_snapshot_;
}

uint64_t RaftStateMachine::last_commit_index() {
    return last_committed_idx_;
}

void RaftStateMachine::create_snapshot(nuraft::snapshot& s, nuraft::async_result<bool>::handler_type& when_done) {
    if (when_done) {
        nuraft::ptr<std::exception> null_ex = nullptr;
        when_done(true, null_ex);
    }
}

} // namespace consensus
