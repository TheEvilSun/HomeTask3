#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

namespace allocators {
namespace fixed {
template <typename T, size_t Capacity>
struct Allocator {
    using value_type = T;

    Allocator() noexcept :
        m_pool(::operator new(sizeof(T) * Capacity), deleter()){
    }


    Allocator(const Allocator& other) = delete;

    Allocator(Allocator&& other) {
        m_pool = std::move(other.m_pool);
        m_count = other.m_count;

        other.m_count = 0;
    }

    Allocator select_on_container_copy_construction() const {
        return Allocator();
    }

    T* allocate(std::size_t n) {
        auto res = static_cast<T*>(m_pool.get() + sizeof(T) * m_count);
        m_count += n;

        if(m_count > Capacity) {
            res = nullptr;
            throw std::overflow_error("Error: try to allocate more than " + std::to_string(Capacity) + " elements!");
        }

        return res;
    }

    void deallocate(T* p, std::size_t n) {
        m_count = 0;
    }

    size_t capacity() const {
        return Capacity;
    }

    size_t allocatedCount() const {
        return m_count;
    }

    template <typename U>
    struct rebind {
        using other = Allocator<U, Capacity>;
    };

    bool operator== (const Allocator& other) const
    {
        if(m_count != other.m_count) {
            return false;
        }
        else if(m_count == 0) {
            return true;
        }

        return memcmp(m_pool.get(), other.m_pool.get(), m_count * sizeof(T)) == 0;
    }

    bool operator!= (const Allocator& other) const
    {
        if(m_count != other.m_count) {
            return true;
        }
        else if(m_count == 0) {
            return false;
        }

        return memcmp(m_pool.get(), other.m_pool.get(), m_count * sizeof(T)) != 0;
    }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

private:
    struct deleter {
        void operator()(void* p) {
            ::operator delete(p);
        }
    };

    std::unique_ptr<void, deleter> m_pool;
    size_t m_count = 0;

};
}

namespace extensible {
template <typename T, size_t PoolSize>
struct Allocator {
    using value_type = T;

    Allocator() :
        m_memoryMask(PoolSize, true){
        m_pools.emplace_back(std::malloc(sizeof(T) * PoolSize), deleter());
    }

    Allocator(size_t poolCount) : m_memoryMask(PoolSize * (poolCount > 0 ? poolCount : 1), true) {
        auto count = poolCount > 0 ? poolCount : 1;
        m_pools.reserve(count);
        for(size_t i = 0; i < count; i++) {
            m_pools.emplace_back(std::malloc(sizeof(T) * PoolSize), deleter());
        }
    }


    Allocator(const Allocator& other) = delete;

    Allocator(Allocator&& other) {
        m_pools = std::move(other.m_pools);
        m_memoryMask = std::move(other.m_memoryMask);
        m_count = other.m_count;
        other.m_count = 0;
    }

    Allocator select_on_container_copy_construction() const {
        return Allocator();
    }

    T* allocate(std::size_t n) {
        if(n > 1) {
            throw std::invalid_argument("Error: try to allocate more than 1 element at a time!");
            return nullptr;
        }

        if(m_count == m_memoryMask.size()) {
            m_pools.emplace_back(std::malloc(sizeof(T) * PoolSize), deleter());
            m_memoryMask.resize(m_memoryMask.size() + PoolSize, true);
        }

        m_count += n;

        for(size_t i = 0; i < m_memoryMask.size(); i++) {
            if(m_memoryMask[i]) {
                m_memoryMask[i] = false;
                return static_cast<T*>(m_pools[i / PoolSize].get()) + (i % PoolSize);
            }
        }


        return nullptr;
    }

    void deallocate(T* p, std::size_t n) {
        if(n > 1) {
            throw std::invalid_argument("Error: try to deallocate more than 1 element at a time!");
            return;
        }

        m_count -= n;
        for(size_t i = 0; i < m_memoryMask.size(); i++) {
            auto pos = static_cast<T*>(m_pools[i / PoolSize].get()) + (i % PoolSize);
            if(!m_memoryMask[i] && pos == p) {
                m_memoryMask[i] = true;
                return;
            }
        }
    }

    size_t capacity() const {
        return m_pools.size() * PoolSize;
    }

    size_t poolSize() const {
        return PoolSize;
    }

    size_t allocatedCount() const {
        return m_count;
    }

    template <typename U>
    struct rebind {
        using other = Allocator<U, PoolSize>;
    };

    bool operator== (const Allocator& other) const
    {
        if(m_count != other.m_count) {
            return false;
        }
        else if(m_count == 0) {
            return true;
        }
        else if(m_pools.size() != other.m_pools.size()){
            return false;
        }
        else {
            for(size_t i = 0; i < m_pools.size(); i++) {
                if(memcmp(m_pools[i].get(), other.m_pools[i].get(), PoolSize * sizeof(T)) != 0) {
                    return false;
                }
            }
        }

        return true;
    }

    bool operator!= (const Allocator& other) const
    {
        if(m_count != other.m_count) {
            return true;
        }
        else if(m_count == 0) {
            return false;
        }
        else if(m_pools.size() != other.m_pools.size()){
            return true;
        }
        else {
            for(size_t i = 0; i < m_pools.size(); i++) {
                if(memcmp(m_pools[i].get(), other.m_pools[i].get(), PoolSize * sizeof(T)) != 0) {
                    return true;
                }
            }
        }

        return false;
    }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

private:
    struct deleter {
        void operator()(void* p) {
            std::free(p);
        }
    };

    std::vector<std::unique_ptr<void, deleter>> m_pools;
    std::vector<bool> m_memoryMask;
    size_t m_count = 0;

};
}
}

template <typename T, typename U, size_t CT, size_t CU>
constexpr bool operator== (const allocators::fixed::Allocator<T, CT>& a1, const allocators::fixed::Allocator<U, CU>& a2) noexcept
{
    return false;
}

template <typename T, typename U, size_t CT, size_t CU>
constexpr bool operator!= (const allocators::fixed::Allocator<T, CT>& a1, const allocators::fixed::Allocator<U, CU>& a2) noexcept
{
    return true;
}

template <typename T, typename U, size_t CT, size_t CU>
constexpr bool operator== (const allocators::extensible::Allocator<T, CT>& a1, const allocators::extensible::Allocator<U, CU>& a2) noexcept
{
    return false;
}

template <typename T, typename U, size_t CT, size_t CU>
constexpr bool operator!= (const allocators::extensible::Allocator<T, CT>& a1, const allocators::extensible::Allocator<U, CU>& a2) noexcept
{
    return true;
}
