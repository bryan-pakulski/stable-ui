#pragma once

#include <fstream>
#include <imgui.h>

#include "../QLogger.h"
#include "../Rendering/RenderManager.h"

class QDisplay_Base {

protected:
  RenderManager *m_renderManager;

public:
  QDisplay_Base(RenderManager *rm) : m_renderManager(rm) {}
  virtual void render() {}
};
