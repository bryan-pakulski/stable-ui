#include "Display/ErrorHandler.h"
#include "Display/QDisplay.h"
#include "Rendering/StableManager.h"
#include "SDInterface/Heartbeat.h"
#include <imgui_impl_glfw.h>
#include <memory>

int main() {

  // Intialise Render manager & attach to Display
  std::shared_ptr<StableManager> rm(new StableManager(*QDisplay::GetInstance().getWindow()));
  QDisplay::GetInstance().AttachManager(rm);

  // Initialise heartbeat
  std::unique_ptr<Heartbeat> heartbeat(new Heartbeat());

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

  heartbeat->stop();

  return 0;
}
