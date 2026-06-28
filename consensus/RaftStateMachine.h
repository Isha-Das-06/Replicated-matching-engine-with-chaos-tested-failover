#pragma once

#include <libnuraft/nuraft.hxx>
#include <mutex>
#include "../matching_engine/MatchingEngine.h"

namespace consensus {

// We serialize an order submission into a buffer
struct RaftOrderPayload {
    uint64_t client_id;
    uint64_t client_seq;
    me::Side side;
    me::OrderType type;
    uint64_t price;
    uint64_t quantity;
};

class RaftStateMachine : public nuraft::state_machine {
public:
    RaftStateMachine(me::MatchingEngine& engine);
    ~RaftStateMachine() = default;

    nuraft::ptr<nuraft::buffer> commit(const uint64_t log_idx, nuraft::buffer& data) override;
    
    nuraft::ptr<nuraft::buffer> commit_config(const uint64_t log_idx, nuraft::ptr<nuraft::cluster_config>& new_conf) override;
    
    void rollback(const uint64_t log_idx, nuraft::buffer& data) override;
    
    int read_logical_snp_obj(nuraft::snapshot& s, void*& user_snp_ctx, unsigned long obj_id, nuraft::ptr<nuraft::buffer>& data_out, bool& is_last_obj) override;
    
    void save_logical_snp_obj(nuraft::snapshot& s, unsigned long& obj_id, nuraft::buffer& data, bool is_first_obj, bool is_last_obj) override;
    
    bool apply_snapshot(nuraft::snapshot& s) override;
    
    void free_user_snp_ctx(void*& user_snp_ctx) override;
    
    nuraft::ptr<nuraft::snapshot> last_snapshot() override;
    
    uint64_t last_commit_index() override;
    
    void create_snapshot(nuraft::snapshot& s, nuraft::async_result<bool>::handler_type& when_done) override;

private:
    me::MatchingEngine& engine_;
    std::mutex engine_mutex_;
    uint64_t last_committed_idx_;
    nuraft::ptr<nuraft::snapshot> last_snapshot_;
};

} // namespace consensus
