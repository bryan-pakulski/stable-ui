#pragma once

#include "Helpers/States.h"
#include "Helpers/Timer.h"
#include <condition_variable>
#include <thread>
#include <chrono>

class Heartbeat {
public:
  int m_state = HEARTBEAT_STATE::DEAD;

public:
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

private:
  std::thread m_thread;
  int m_lastState = HEARTBEAT_STATE::POLL;
  const int c_timer = 5;
  timer_killer m_kill_timer = timer_killer();

private:
  Heartbeat() {}
  ~Heartbeat() {}

  void run();
};