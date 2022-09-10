#include "src/Display/ErrorHandler.h"
#include "src/Display/QDisplay.h"
#include "src/Rendering/RenderManager.h"

int main() {

  RenderManager rm(*QDisplay::GetInstance().getWindow());
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

    // Render and catch events
    QDisplay::processFrameAndEvents();
  }

  return 0;
}
