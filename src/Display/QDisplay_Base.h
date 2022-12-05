#pragma once

#include <fstream>
#include <imgui.h>

#include "../QLogger.h"
#include "../Rendering/RenderManager.h"

// Basic menu class
// QDisplay uses this base class as a reference via smart pointer, this is to
// call the overloaded virtual render function

struct listItem {
  std::string m_name;
  bool m_isSelected = false;
};
class QDisplay_Base {
  GLFWwindow *m_window;

protected:
  std::shared_ptr<RenderManager> m_renderManager;

  void getWindowSize(std::pair<int, int> &size) { glfwGetFramebufferSize(m_window, &size.first, &size.second); }

public:
  QDisplay_Base(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : m_renderManager(rm), m_window(w) {}
  virtual void render() {}
};
