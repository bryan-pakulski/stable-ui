#pragma once

#include <fstream>
#include <imgui.h>

#include "Helpers/QLogger.h"
#include "Rendering/RenderManager.h"

// Basic menu class
// QDisplay uses this base class as a reference via smart pointer, this is to
// call the overloaded virtual render function

struct listItem {
  std::string m_name;
  std::string m_key;
  bool m_isSelected = false;
};
class QDisplay_Base {

public:
  bool m_isOpen = false;

public:
  QDisplay_Base(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : m_renderManager(rm), m_window(w) {}
  virtual void render() {}

protected:
  std::shared_ptr<RenderManager> m_renderManager;
  GLFWwindow *m_window;

protected:
  void getWindowSize(std::pair<int, int> &size) { glfwGetFramebufferSize(m_window, &size.first, &size.second); }
};
