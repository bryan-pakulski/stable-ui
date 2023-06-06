#include "Display/ErrorHandler.h"
#include "Display/QDisplay.h"
#include "StableManager.h"
#include "Client/StableClient.h"
#include "Client/Heartbeat.h"
#include <imgui_impl_glfw.h>
#include <memory>

int main() {

  // Initialise display and logic manager
  QDisplay::GetInstance().AttachManager(StableManager::GetInstance().getRenderManager());

  // Initialise heartbeat to docker
  StableClient::GetInstance();
  Heartbeat::GetInstance().start();

  while (!glfwWindowShouldClose(QDisplay::GetInstance().getWindow())) {

    // Clean OpenGL frame & imgui interface
    QDisplay::clearFrame();

    // Only render if no errors detected
    if (!ErrorHandler::GetInstance().hasError()) {
      // UI logic
      QDisplay::GetInstance().drawMenus();

      // Stable Manager loop (rendering)
      StableManager::GetInstance().update();
    }

    // Display any captured errors as a modal popup over the top of the screen
    ErrorHandler::GetInstance().pollErrors();

    // Process and catch events, draw ui
    QDisplay::processFrameAndEvents();
  }

  Heartbeat::GetInstance().stop();

  return 0;
}
