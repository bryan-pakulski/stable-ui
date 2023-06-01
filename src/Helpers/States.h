#pragma once

#include <string>

#ifndef _EXEC_STATE
#define _EXEC_STATE

// Docker command execution
struct Q_COMMAND_EXECUTION_STATE {
  const static int FAILED = -1;
  const static int PENDING = 0;
  const static int SUCCESS = 1;
};

// Docker status
struct Q_DOCKER_STATUS {
  const static int DISCONNECTED = -1;
  const static int CONNECTED = 1;
};

// Model state in server
struct Q_MODEL_STATUS {
  const static int FAILED = -1;
  const static int NONE_LOADED = 0;
  const static int LOADING = 1;
  const static int LOADED = 2;
};

// Render status
struct Q_RENDER_STATE {
  const static int UNRENDERED = 0;
  const static int RENDERING = 1;
  const static int RENDERED = 2;
};

// Heartbeat status
struct HEARTBEAT_STATE {
  const static int DEAD = -1;
  const static int POLL = 0;
  const static int ALIVE = 1;
};

#endif
