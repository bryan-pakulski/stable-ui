#pragma once

#include <imgui.h>
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"

#include <imgui_stdlib.h>

class QDisplay_LogWindow : public QDisplay_Base {
public:
  QDisplay_LogWindow(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  void openWindow(){};

  virtual void render() {}

private:
};