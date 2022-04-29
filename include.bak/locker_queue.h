#pragma once

#include "shm_buf_locker.h"
#include <atomic>
#include <iostream>
#include <stdint.h>
#include <thread>

namespace ipc {
namespace shm {

template <uint64_t size = 1000> class LockerQueue {
public:
  bool LockPush(const LockedInfo &locked_info, uint64_t &index);

  bool UnlockPush(uint64_t index);

  bool LockAt(uint64_t index, LockedInfo &locked_info);

  bool UnlockAt(uint64_t index);

  uint64_t Capacity() const { return size; }

private:
  void Pop(const LockedInfo &locked_info);

private:
  std::atomic<uint64_t> head_index_ = {0};
  std::atomic<uint64_t> sequece_ = {0};
  std::atomic<uint64_t> count_ = {0};
  ShmBufLocker lockers_[size] = {};
};

template <uint64_t size>
bool LockerQueue<size>::LockPush(const LockedInfo &locked_info,
                                 uint64_t &index) {
  Pop(locked_info);
  uint64_t seq = sequece_.load();
  ShmBufLocker *locker = nullptr;
  do {
    locker = &lockers_[seq % size];
  } while (!sequece_.compare_exchange_weak(seq, seq + 1));
  count_.fetch_add(1);
  while (!locker->TryLockForWrite()) {
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  locker->Set(locked_info);
  // count_.fetch_add(1);
  index = seq % size;
  return true;
}

template <uint64_t size>
void LockerQueue<size>::Pop(const LockedInfo &locked_info) {
  ShmBufLocker *head_locker = nullptr;
  while (true) {
    auto head_index = head_index_.load();
    do {
      head_locker = &lockers_[head_index];
      uint64_t count = count_.load();
      do {
        if (count == 0) {
          return;
        } else if (count == size ||
                   (head_locker->LockedOffset() >= locked_info.locked_offset &&
                    head_locker->LockedOffset() + head_locker->LockedSize() >
                        locked_info.locked_offset)) {
          continue;
        }
        return;
      } while (!count_.compare_exchange_weak(count, count - 1));
    } while (!head_index_.compare_exchange_weak(head_index,
                                                (head_index + 1) % size));
    while (!head_locker->TryLockForWrite()) {
      std::cout << "LockerQueue<size>::Pop head_locker->TryLockForWrite failed."
                << std::endl;
    }
    head_locker->Reset();
    head_locker->ReleaseWriteLock();
  }
}

template <uint64_t size> bool LockerQueue<size>::UnlockPush(uint64_t index) {
  if (index >= size) {
    std::cout << "invalid parameter for unlockpush index:" << index
              << std::endl;
    return false;
  }

  ShmBufLocker *locker = &lockers_[index];
  locker->ReleaseWriteLock();
  return true;
}

template <uint64_t size>
bool LockerQueue<size>::LockAt(uint64_t index, LockedInfo &locked_info) {
  if (index >= size) {
    std::cout << "invalid parameter index:" << index << std::endl;
    return false;
  }

  ShmBufLocker *locker = &lockers_[index];
  if (!locker->TryLockForRead()) {
    std::cout << "index:" << index << " is not allowed now." << std::endl;
    return false;
  }
  if (!locker->Setted()) {
    std::cout << "index:" << index << " is not setted." << std::endl;
    locker->ReleaseReadLock();
    return false;
  }
  locked_info.locked_offset = locker->LockedOffset();
  locked_info.locked_size = locker->LockedSize();
  return true;
}

template <uint64_t size> bool LockerQueue<size>::UnlockAt(uint64_t index) {
  if (index >= size) {
    std::cout << "invalid parameter for unlockat index:" << index << std::endl;
    return false;
  }
  ShmBufLocker *locker = &lockers_[index];
  locker->ReleaseReadLock();
  return true;
}
}
}
