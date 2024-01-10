#pragma once

#include <memory>
#include <optional>

namespace containers {
template<typename T, typename Allocator = std::allocator<T>>
class List {
    struct Node {
        T value;
        Node* next = nullptr;

        Node(const T& v, Node* n) : value(v), next(n) {}
    };

    Node* m_head = nullptr;
    Node* m_current = nullptr;
    using NodeAllocator = typename Allocator::template rebind<Node>::other;
    NodeAllocator m_allocator;

public:
    void append(const T& value) {
        auto appendNode = std::allocator_traits<NodeAllocator>::allocate(m_allocator, 1);
        std::allocator_traits<NodeAllocator>::construct(m_allocator, appendNode, value, nullptr);

        if(m_head) {
            auto last = m_head;
            while(last->next) {
                last = last->next;
            }           
            last->next = appendNode;
            if(!m_current) {
                m_current = appendNode;
            }
        }
        else {
            m_head = appendNode;
            m_current = m_head;
        }
    }

    std::optional<T> getValue() {
        if(m_current) {
            auto value = m_current->value;
            m_current = m_current->next;
            return std::make_optional<T>(std::move(value));
        }
        else {
            return {};
        }
    }

    void backToHead() {
        m_current = m_head;
    }

    bool onHead() const {
        return m_head == m_current;
    }

};
}
