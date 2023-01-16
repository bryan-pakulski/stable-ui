#include "Display/ErrorHandler.h"
#include "Display/QDisplay.h"
#include "Rendering/RenderManager.h"
#include <imgui_impl_glfw.h>

int main() {

  // Intialise Render manager & attach to Display
  std::shared_ptr<RenderManager> rm(new RenderManager(*QDisplay::GetInstance().getWindow()));
  QDisplay::GetInstance().AttachRenderManager(rm);

  while (!glfwWindowShouldClose(QDisplay::GetInstance().getWindow())) {

    // Clean OpenGL frame & imgui interface
    QDisplay::clearFrame();

    // Only render if no errors detected
    if (!ErrorHandler::GetInstance().hasError()) {
      // Sub menus rendering & logic
      QDisplay::GetInstance().drawMenus();

      // Rendering loop for canvas
      rm->update();
    }

    // Display any captured errors as a modal popup over the top of the screen
    ErrorHandler::GetInstance().pollErrors();

    // Process and catch events
    QDisplay::processFrameAndEvents();
  }

  return 0;
}
