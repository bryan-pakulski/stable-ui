#include "Heartbeat.h"
#include "../../../Display/ErrorHandler.h"

// TODO: make this a singleton so that the connection can be monitored via the display toolbar

Heartbeat::Heartbeat() {}

Heartbeat::~Heartbeat() {}

void Heartbeat::start() {
  m_thread = std::thread([=] { run(); });
}
void Heartbeat::stop() {
  m_kill_timer.kill();
  m_thread.join();
}

void Heartbeat::run() {
  while (m_kill_timer.wait_for(std::chrono::seconds(c_timer))) {

    // Call sd-client to ping sd-server
    SDCommandsInterface::GetInstance().heartbeat(m_state);

    if (m_state == HEARTBEAT_STATE::DEAD && m_lastState != HEARTBEAT_STATE::DEAD) {
      ErrorHandler::GetInstance().setError("No heartbeat to SD Server, is docker running?");
    }

    // Only fire an error once on timeout to avoid spamming
    m_lastState = m_state;
  }
}

int Heartbeat::getState() {
  if (m_state == HEARTBEAT_STATE::DEAD) {
    return 0;
  } else {
    return 1;
  }
}