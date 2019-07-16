#pragma once

#include <atomic>
#include <memory>

template <typename T> class LockFreeQueue {

  struct node;
  struct counted_node_ptr {
    int external_count;
    node *ptr;
  };

  struct node_counter {
    unsigned internal_count : 30;
    unsigned external_counters : 2; // 2
  };

  struct node {
    std::atomic<T *> data;
    std::atomic<node_counter> count; // 3
    std::atomic<counted_node_ptr> next;

    node() {
      node_counter new_count;
      new_count.internal_count = 0;
      new_count.external_counters = 2; // 4
      count.store(new_count);

      next.ptr = nullptr;
      next.external_count = 0;
    }
    void release_ref() {
      node_counter old_counter = count.load(std::memory_order_relaxed);
      node_counter new_counter;
      do {
        new_counter = old_counter;
        --new_counter.internal_count;          // 1
      } while (!count.compare_exchange_strong( // 2
          old_counter, new_counter, std::memory_order_acquire,
          std::memory_order_relaxed));
      if (!new_counter.internal_count && !new_counter.external_counters) {
        delete this; // 3
      }
    };
  };

public:
  void push(T new_value) {
    std::unique_ptr<T> new_data(new T(new_value));
    counted_node_ptr new_next;
    new_next.ptr = new node;
    new_next.external_count = 1;
    counted_node_ptr old_tail = tail.load();

    for (;;) {
      increase_external_count(tail, old_tail);

      T *old_data = nullptr;
      if (old_tail.ptr->data.compare_exchange_strong( // 6
              old_data, new_data.get())) {
        counted_node_ptr old_next = {0};
        if (!old_tail.ptr->next.compare_exchange_strong( // 7
                old_next, new_next)) {
          delete new_next.ptr; // 8
          new_next = old_next; // 9
        }
        set_new_tail(old_tail, new_next);
        new_data.release();
        break;
      } else // 10
      {
        counted_node_ptr old_next = {0};
        if (old_tail.ptr->next.compare_exchange_strong( // 11
                old_next, new_next)) {
          old_next = new_next;     // 12
          new_next.ptr = new node; // 13
        }
        set_new_tail(old_tail, old_next); // 14
      }
    }
  }
}

std::unique_ptr<T>
pop() {
  counted_node_ptr old_head = head.load(std::memory_order_relaxed);
  for (;;) {
    increase_external_count(head, old_head);
    node *const ptr = old_head.ptr;
    if (ptr == tail.load().ptr) {
      return std::unique_ptr<T>();
    }
    counted_node_ptr next = ptr->next.load(); // 2
    if (head.compare_exchange_strong(old_head, next)) {
      T *const res = ptr->data.exchange(nullptr);
      free_external_counter(old_head);
      return std::unique_ptr<T>(res);
    }
    ptr->release_ref();
  }
}

private:
static void increase_external_count(std::atomic<counted_node_ptr> &counter,
                                    counted_node_ptr &old_counter) {
  counted_node_ptr new_counter;
  do {
    new_counter = old_counter;
    ++new_counter.external_count;
  } while (!counter.compare_exchange_strong(old_counter, new_counter,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed));

  old_counter.external_count = new_counter.external_count;
}

static void free_external_counter(counted_node_ptr &old_node_ptr) {
  node *const ptr = old_node_ptr.ptr;
  int const count_increase = old_node_ptr.external_count - 2;

  node_counter old_counter = ptr->count.load(std::memory_order_relaxed);
  node_counter new_counter;
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

void set_new_tail(counted_node_ptr &old_tail, // 1
                  counted_node_ptr const &new_tail) {
  node *const current_tail_ptr = old_tail.ptr;
  while (!tail.compare_exchange_weak(old_tail, new_tail) && // 2
         old_tail.ptr == current_tail_ptr)
    ;
  if (old_tail.ptr == current_tail_ptr) // 3
    free_external_counter(old_tail);    // 4
  else
    current_tail_ptr->release_ref(); // 5
}

private:
std::atomic<counted_node_ptr> head;
std::atomic<counted_node_ptr> tail; // 1
}
;