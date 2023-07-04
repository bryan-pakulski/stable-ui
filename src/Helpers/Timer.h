#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

struct timer_killer {
  // returns false if killed:
  template <class R, class P> bool wait_for(std::chrono::duration<R, P> const &time) const {
    std::unique_lock<std::mutex> lock(m);
    return !cv.wait_for(lock, time, [&] { return terminate; });
  }

  void kill() {
    std::unique_lock<std::mutex> lock(m);
    terminate = true;
    cv.notify_all();
  }

  void update() {
    std::unique_lock<std::mutex> lock(m);
    cv.notify_all();
  }

  timer_killer() = default;
  timer_killer(timer_killer &&) = delete;
  timer_killer(timer_killer const &) = delete;
  timer_killer &operator=(timer_killer &&) = delete;
  timer_killer &operator=(timer_killer const &) = delete;

private:
  mutable std::condition_variable cv;
  mutable std::mutex m;
  bool terminate = false;
};
