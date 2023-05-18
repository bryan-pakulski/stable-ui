#pragma once

#include "SDCommandsInterface.h"
#include "../Helpers/States.h"
#include <condition_variable>
#include <thread>
#include <chrono>

struct timer_killer {
  // returns false if killed:
  template <class R, class P> bool wait_for(std::chrono::duration<R, P> const &time) const {
    std::unique_lock<std::mutex> lock(m);
    return !cv.wait_for(lock, time, [&] { return terminate; });
  }
  void kill() {
    std::unique_lock<std::mutex> lock(m);
    terminate = true; // should be modified inside mutex lock
    cv.notify_all();  // it is safe, and *sometimes* optimal, to do this outside the lock
  }
  // I like to explicitly delete/default special member functions:
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

class Heartbeat {
private:
  std::thread m_thread;
  int m_lastState = HEARTBEAT_STATE::POLL;
  const int c_timer = 5;
  timer_killer m_kill_timer = timer_killer();

  Heartbeat();
  ~Heartbeat();

  void run();

public:
  int m_state = HEARTBEAT_STATE::DEAD;

  static Heartbeat &GetInstance() {
    static Heartbeat s_hb;
    return s_hb;
  }

  // Prevent replication
  Heartbeat(Heartbeat const &) = delete;
  void operator=(Heartbeat const &) = delete;

  void start();
  void stop();
  int getState();
};