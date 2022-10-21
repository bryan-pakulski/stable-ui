#pragma once

#include <fstream>
#include <imgui.h>

#include "../QLogger.h"
#include "../Rendering/RenderManager.h"

// Basic menu class
// QDisplay uses this base class as a reference via smart pointer, this is to
// call the overloaded virtual render function
class QDisplay_Base {

protected:
   std::shared_ptr<RenderManager> m_renderManager;

public:
  QDisplay_Base(std::shared_ptr<RenderManager> rm) : m_renderManager(rm) {}
  virtual void render() {}
};