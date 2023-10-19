#pragma once
#include <condition_variable>
#include <iostream>
#include <queue>
#include <mutex>

// Thread-Safe Queue using condiditional variables
// https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html

template <typename Data> class asyncQueue {

public:
  bool killed = false;

public:
  void kill() {
    std::unique_lock<std::mutex> lock(m_mutex);
    killed = true;
    m_conditionVariable.notify_one();
  }

  void push(Data const &data) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_queue.push(data);
    m_conditionVariable.notify_one();
  }

  bool empty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  bool try_pop(Data &popped_value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
      return false;
    }

    popped_value = m_queue.front();
    m_queue.pop();
    return true;
  }

  void wait_and_pop(Data &popped_value) {
    std::unique_lock<std::mutex> lock(m_mutex);

    // Stop waiting when we have data in the queue or we have received a kill signal
    m_conditionVariable.wait(lock, [this] { return !m_queue.empty() || killed; });
    if (killed) {
      return;
    }

    popped_value = m_queue.front();
    m_queue.pop();
  }

private:
  std::queue<Data> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_conditionVariable;
};