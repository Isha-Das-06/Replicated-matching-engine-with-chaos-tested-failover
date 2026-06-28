#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

namespace me {

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t capacity) : capacity_(capacity) {
        pool_.resize(capacity);
        free_list_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            free_list_.push_back(&pool_[capacity - 1 - i]);
        }
    }

    T* allocate() {
        if (free_list_.empty()) {
            throw std::bad_alloc(); // In a real system, you might expand the pool instead
        }
        T* obj = free_list_.back();
        free_list_.pop_back();
        return obj;
    }

    void deallocate(T* obj) {
        free_list_.push_back(obj);
    }

private:
    size_t capacity_;
    std::vector<T> pool_;
    std::vector<T*> free_list_;
};

} // namespace me
