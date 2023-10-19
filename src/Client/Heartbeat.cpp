#include "Heartbeat.h"
#include "Display/ErrorHandler.h"
#include "Client/StableClient.h"

void Heartbeat::start() {
  m_thread = std::thread([this] { run(); });
}
void Heartbeat::stop() {
  m_kill_timer.kill();
  m_thread.join();
}

void Heartbeat::run() {
  while (m_kill_timer.wait_for(std::chrono::seconds(c_timer))) {

    // Call sd-client to ping sd-server
    QLogger::GetInstance().Log(LOGLEVEL::DBG1, "Heartbeat - ping");
    StableClient::GetInstance().heartbeat(m_state);

    // Only throw an error when we experience a connection failure, on initial connect the last_state is POLL
    if (m_state == HEARTBEAT_STATE::DEAD && m_lastState == HEARTBEAT_STATE::ALIVE) {
      ErrorHandler::GetInstance().setError("Docker Heartbeat", "No heartbeat to SD Server, is docker running?");
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "lost heartbeat with docker");
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