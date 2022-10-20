#include "src/Display/ErrorHandler.h"
#include "src/Display/QDisplay.h"
#include "src/Rendering/RenderManager.h"
#include <imgui_impl_glfw.h>
int main() {

  // Intialise Render manager & attach to Display
  RenderManager rm(*QDisplay::GetInstance().getWindow());
  QDisplay::GetInstance().AttachRenderManager(&rm);

  while (!glfwWindowShouldClose(QDisplay::GetInstance().getWindow())) {

    QDisplay::clearFrame();

    // Display any captured errors as a modal popup over the top of the screen
    if (ErrorHandler::GetInstance().hasError()) {
      ErrorHandler::GetInstance().displayError();
    }

    // Sub menus rendering & logic
    QDisplay::GetInstance().drawMenus();

    // Rendering loop for objects
    rm.update();

    // Process and catch events
    QDisplay::processFrameAndEvents();
  }

  return 0;
}
