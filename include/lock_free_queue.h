#pragma once

#include <atomic>
#include <memory>

template <typename T> class LockFreeQueue {

  struct Node;
  struct CountedNodePtr {
    int external_count; //节点外部计数
    Node *ptr;
  };

  struct NodeCounter {
    unsigned internal_count : 30;   //节点内部计数
    unsigned external_counters : 2; //节点外部计数器个数
  };

  struct Node {
    std::atomic<T *> data;
    std::atomic<NodeCounter> count; // 3
    std::atomic<CountedNodePtr> next;

    Node() {
      NodeCounter new_count;
      new_count.internal_count = 0;
      new_count.external_counters = 2; // 4
      count.store(new_count);

      next.ptr = nullptr;
      next.external_count = 0;
    }

    void ReleaseRef() {
      NodeCounter old_counter = count.load(std::memory_order_relaxed);
      NodeCounter new_counter;
      do {
        new_counter = old_counter;
        --new_counter.internal_count;          // 1
      } while (!count.compare_exchange_strong( // 2
          old_counter, new_counter, std::memory_order_acquire,
          std::memory_order_relaxed));
      if (!new_counter.internal_count && !new_counter.external_counters) {
        delete this; // 3
      }
    }
  };

public:
  void Push(T new_value) {
    std::unique_ptr<T> new_data(new T(new_value));
    CountedNodePtr new_next;
    new_next.ptr = new Node;
    new_next.external_count = 1;
    CountedNodePtr old_tail = tail_.load();

    for (;;) {
      IncreaseExternalCount(tail_, old_tail);

      T *old_data = nullptr;
      if (old_tail.ptr->data.compare_exchange_strong( // 6
              old_data, new_data.get())) {
        CountedNodePtr old_next = {0};
        if (!old_tail.ptr->next.compare_exchange_strong( // 7
                old_next, new_next)) {
          delete new_next.ptr; // 8
          new_next = old_next; // 9
        }
        SetNewTail(old_tail, new_next);
        new_data.release();
        break;
      } else // 10
      {
        CountedNodePtr old_next = {0};
        if (old_tail.ptr->next.compare_exchange_strong( // 11
                old_next, new_next)) {
          old_next = new_next;     // 12
          new_next.ptr = new Node; // 13
        }
        SetNewTail(old_tail, old_next); // 14
      }
    }
  }

  std::unique_ptr<T> Pop() {
    CountedNodePtr old_head = head_.load(std::memory_order_relaxed);
    for (;;) {
      IncreaseExternalCount(head_, old_head);
      Node *const ptr = old_head.ptr;
      if (ptr == tail_.load().ptr) {
        return std::unique_ptr<T>();
      }
      CountedNodePtr next = ptr->next.load(); // 2
      if (head_.compare_exchange_strong(old_head, next)) {
        T *const res = ptr->data.exchange(nullptr);
        FreeExternalCounter(old_head);
        return std::unique_ptr<T>(res);
      }
      ptr->ReleaseRef();
    }
  }

private:
  static void IncreaseExternalCount(std::atomic<CountedNodePtr> &counter,
                                    CountedNodePtr &old_counter) {
    CountedNodePtr new_counter;
    do {
      new_counter = old_counter;
      ++new_counter.external_count;
    } while (!counter.compare_exchange_strong(old_counter, new_counter,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed));

    old_counter.external_count = new_counter.external_count;
  }

  static void FreeExternalCounter(CountedNodePtr &old_node_ptr) {
    Node *const ptr = old_node_ptr.ptr;
    int const count_increase = old_node_ptr.external_count - 2;

    NodeCounter old_counter = ptr->count.load(std::memory_order_relaxed);
    NodeCounter new_counter;
    do {
      new_counter = old_counter;
      --new_counter.external_counters;              // 1
      new_counter.internal_count += count_increase; // 2
    } while (!ptr->count.compare_exchange_strong(   // 3
        old_counter, new_counter, std::memory_order_acquire,
        std::memory_order_relaxed));

    if (!new_counter.internal_count && !new_counter.external_counters) {
      delete ptr; // 4
    }
  }

  void SetNewTail(CountedNodePtr &old_tail, // 1
                  CountedNodePtr const &new_tail) {
    Node *const current_tail_ptr = old_tail.ptr;
    while (!tail_.compare_exchange_weak(old_tail, new_tail) && // 2
           old_tail.ptr == current_tail_ptr)
      ;
    if (old_tail.ptr == current_tail_ptr) // 3
      FreeExternalCounter(old_tail);      // 4
    else
      current_tail_ptr->ReleaseRef(); // 5
  }

private:
  std::atomic<CountedNodePtr> head_;
  std::atomic<CountedNodePtr> tail_; // 1
};