#pragma once

#include "../QLogger.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <vector>

#include "../Rendering/StableManager.h"
#include "../Config/config.h"
#include "Menus/BottomBar/QDisplay_BottomBar.h"
#include "Menus/TopBar/QDisplay_TopBar.h"
#include "Menus/LeftBar/QDisplay_LeftBar.h"
#include "QDisplay_Base.h"

#include "Themes.h"

// Singleton display class
class QDisplay {
public:
  static QDisplay &GetInstance() {
    static QDisplay s_display;
    return s_display;
  }

  // Attach a render manager instance
  // This is necessary for sub menus to interact with the render manager
  void AttachManager(std::shared_ptr<StableManager> rm) {
    m_stableManager = rm;

    // Initialisation
    m_submenus.emplace_back(new QDisplay_TopBar(m_stableManager, m_window));
    m_submenus.emplace_back(new QDisplay_BottomBar(m_stableManager, m_window));
    m_submenus.emplace_back(new QDisplay_LeftBar(m_stableManager, m_window));

    // Enable debug output
    if (CONFIG::ENABLE_GL_DEBUG.get() == 1) {
      glEnable(GL_DEBUG_OUTPUT);
      glDebugMessageCallback(m_stableManager->MessageCallback, 0);
    }
  }

  // Return currently active window
  GLFWwindow *getWindow() { return m_window; }

  // Draw all submenus
  void drawMenus() {

    // Dynamically render all windows
    for (auto &menu : m_submenus) {
      menu->render();
    }
  }

  // Clears frame
  static void clearFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  // Render & catch events
  static void processFrameAndEvents() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(QDisplay::GetInstance().getWindow());
    glfwPollEvents();
  }

private:
  GLFWwindow *m_window;
  std::string m_glsl_version;
  std::vector<std::unique_ptr<QDisplay_Base>> m_submenus;
  std::shared_ptr<StableManager> m_stableManager = 0;

  float backgroundR = 0.45f;
  float backgroundG = 0.44f;
  float backgroundB = 0.48f;

  // Window resize callback
  static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {}

  // Cleans up all GL variables for clean exit
  void cleanupDisplay() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  QDisplay() {

    if (!glfwInit()) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Couldn't initialize GLFW");
    } else {
      QLogger::GetInstance().Log(LOGLEVEL::INFO, "GLFW initialized");
    }

    // setup GLFW window
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    m_glsl_version = "#version 150";
    glfwWindowHint( // required on Mac OS
        GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif __linux__
    // GL 3.2 + GLSL 150
    m_glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif _WIN32
    // GL 3.0 + GLSL 130
    m_glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif

#ifdef _WIN32
    // if it's a HighDPI monitor, try to scale everything
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    if (xscale > 1 || yscale > 1) {
      CONFIG::HIGH_DPI_SCALE_FACTOR = xscale;
      glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }
#elif __APPLE__
    // to prevent 1200x800 from becoming 2400x1600
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    m_window = glfwCreateWindow(CONFIG::WINDOW_WIDTH.get(), CONFIG::WINDOW_WIDTH.get(), CONFIG::PROGRAM_NAME.c_str(),
                                nullptr, nullptr);

    if (!m_window) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Couldn't create a GLFW window");
      cleanupDisplay();
    }

    // Catch window resizing
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwMakeContextCurrent(m_window);

    // VSync
    glfwSwapInterval(1);

    // Initialise callbacks for glfw *MUST BE DONE BEFORE IMGUI OTHERWISE IT WILL OVERRIDE THE CALLBACKS THERE*
    glfwSetErrorCallback(StableManager::GLFWErrorCallBack);
    glfwSetKeyCallback(m_window, StableManager::key_callback);
    glfwSetCursorPosCallback(m_window, StableManager::mouse_callback);
    glfwSetMouseButtonCallback(m_window, StableManager::mouse_btn_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    // TODO: make configurable?
    embraceTheDarkness();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(m_glsl_version.c_str());

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Couldn't initialize GLAD");
      cleanupDisplay();
    } else {
      QLogger::GetInstance().Log(LOGLEVEL::INFO, "GLAD initialized");
    }

    // Set clear colour
    glClearColor(backgroundR, backgroundG, backgroundB, 1.0f);

    // Disable ini file configuration from being generated
    // io.IniFilename = NULL;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
  }

  ~QDisplay() { cleanupDisplay(); }
};