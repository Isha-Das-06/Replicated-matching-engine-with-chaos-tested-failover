#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

namespace nuraft {

using ulong = unsigned long;

template<typename T>
using ptr = std::shared_ptr<T>;

class buffer {
public:
    static ptr<buffer> alloc(size_t sz) {
        return std::make_shared<buffer>(sz);
    }
    buffer(size_t sz) { data_.resize(sz); }
    
    void* data_begin() { return data_.data(); }
    size_t size() const { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

class cluster_config {};

class snapshot {};

class clone_task {};

template<typename T>
struct async_result {
    using handler_type = std::function<void(T, ptr<std::exception>)>;
};

class state_machine {
public:
    virtual ~state_machine() = default;
    
    virtual ptr<buffer> commit(const uint64_t log_idx, buffer& data) = 0;
    virtual ptr<buffer> commit_config(const uint64_t log_idx, ptr<cluster_config>& new_conf) = 0;
    virtual void rollback(const uint64_t log_idx, buffer& data) = 0;
    virtual int read_logical_snp_obj(snapshot& s, void*& user_snp_ctx, ulong obj_id, ptr<buffer>& data_out, bool& is_last_obj) = 0;
    virtual void save_logical_snp_obj(snapshot& s, ulong& obj_id, buffer& data, bool is_first_obj, bool is_last_obj) = 0;
    virtual bool apply_snapshot(snapshot& s) = 0;
    virtual void free_user_snp_ctx(void*& user_snp_ctx) = 0;
    virtual ptr<snapshot> last_snapshot() = 0;
    virtual uint64_t last_commit_index() = 0;
    virtual void create_snapshot(snapshot& s, async_result<bool>::handler_type& when_done) = 0;
};

} // namespace nuraft
