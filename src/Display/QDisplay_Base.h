#pragma once

#include <fstream>
#include <imgui.h>

#include "../QLogger.h"
#include "../Rendering/StableManager.h"

// Basic menu class
// QDisplay uses this base class as a reference via smart pointer, this is to
// call the overloaded virtual render function

struct listItem {
  std::string m_name;
  std::string m_key;
  bool m_isSelected = false;
};
class QDisplay_Base {

protected:
  std::shared_ptr<StableManager> m_stableManager;
  GLFWwindow *m_window;

  void getWindowSize(std::pair<int, int> &size) { glfwGetFramebufferSize(m_window, &size.first, &size.second); }

public:
  bool m_isOpen = false;
  QDisplay_Base(std::shared_ptr<StableManager> rm, GLFWwindow *w) : m_stableManager(rm), m_window(w) {}
  virtual void render() {}
};
