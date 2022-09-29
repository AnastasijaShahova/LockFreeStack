#ifndef LOCKFREESTACK_H
#define LOCKFREESTACK_H
#include <iostream>
#include <atomic>

template<typename T>
class LockFreeStack {
private:
    enum Header {
        HEAD, FREE
    };

    struct Node {
        T value;
        Node* next;
    };

    struct Data {

        std::shared_ptr<Node[]> nodes;
        std::pair<Node*, Node*> heads = { nullptr, nullptr};

        Data() {}
        Data(const Data& data_) {
            nodes = std::shared_ptr<Node[]>(data_.nodes);
            heads.first = data_.heads.first;
            heads.second = data_.heads.second;
        }
    };

    struct DataHeader {
        Data* data = nullptr;
        int tag = 0;

        Node*& head() { return data->heads.first; }
        Node*& free() { return data->heads.second; }
    };

    std::atomic<DataHeader> header_;
    int capacity_;

public:

    LockFreeStack(int capacity)  {
        capacity_ = capacity;

        DataHeader header;
        header.data = new Data();
        m_InitData(*header.data, capacity_);

        header_.store(header, std::memory_order_relaxed);
    }

    ~LockFreeStack() {
        DataHeader header = header_.load(std::memory_order_relaxed);
        delete header.data;
    }

    void Push(T value) {
        Node* node = Pop_(FREE);
        if (node == nullptr) {
            std::cout << "Push() called on full stack";
            exit(EXIT_FAILURE);
        }

        node->value = std::move(value);
        Push_(HEAD, node);

    }

    T Pop() {
        Node* node = Pop_(HEAD);
        if (node == nullptr) {
            std::cout << "Pop() called on empty stack";
            exit(EXIT_FAILURE);
        }

        T value = std::move(node->value);
        Push_(FREE, node);

        return value;
    }

    bool IsEmpty() {
        DataHeader header = header_.load(std::memory_order_relaxed);
        return header.head() == nullptr;
    }

    void Clear() {

        DataHeader last = header_.load(std::memory_order_relaxed);
        DataHeader next;
        next.data = new Data();
        m_InitData(*next.data, capacity_);

        do {
            next.tag = last.tag + 1;
        } while (!m_CompareHeader(header_, last, next));

        delete last.data;
    };

private:

    void m_InitData(Data& data, int capacity) {

        data.nodes.reset(new Node[capacity]);
        data.nodes[capacity - 1].next = nullptr;

        for (int i = 0; i < capacity - 1; i++)
            data.nodes[i].next = &data.nodes[i + 1];

        data.heads.first = nullptr;
        data.heads.second = &data.nodes[0];
    }

    bool m_CompareHeader(std::atomic<DataHeader>& a, DataHeader& b, DataHeader& value) {
        return a.compare_exchange_weak(b, value, std::memory_order_acq_rel, std::memory_order_acquire);
    }

    Node* Pop_(Header idx) {
        DataHeader last = header_.load(std::memory_order_relaxed);
        DataHeader next;

        next.data = new Data(*last.data);

        switch(idx) {
        case FREE: {
            do {
                if (last.free() == nullptr) {
                    delete next.data;
                    return nullptr;
                }
                next.tag = last.tag + 1;
                next.free() = last.free()->next;
            } while (!m_CompareHeader(header_, last, next));

            delete last.data;
            return last.free();
        }
        case HEAD: {
            do {
                if (last.head() == nullptr) {
                    delete next.data;
                    return nullptr;
                }
                next.tag = last.tag + 1;
                next.head() = last.head()->next;
            } while (!m_CompareHeader(header_, last, next));

            delete last.data;
            return last.head();
        }
        }
    }

    void Push_(Header idx, Node* node) {

        DataHeader last = header_.load(std::memory_order_relaxed);
        DataHeader next;
        next.data = new Data(*last.data);

        switch(idx) {
        case HEAD: {
            do {
                node->next = last.head();
                next.tag = last.tag + 1;
                next.head() = node;
            } while (!m_CompareHeader(header_, last, next));

            delete last.data;
            break;
        }
        case FREE: {
            do {
                node->next = last.free();
                next.tag = last.tag + 1;
                next.free() = node;
            } while (!m_CompareHeader(header_, last, next));
            delete last.data;
            break;
        }
        }
    }
};

#endif // LOCKFREESTACK_H
