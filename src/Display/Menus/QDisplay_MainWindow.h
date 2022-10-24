#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "../../Display/ErrorHandler.h"
#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"
#include "../../Config/config.h"
#include "../QDisplay_Base.h"

namespace fs = std::filesystem;

struct listItem {
  std::string m_name;
  bool m_isSelected = false;
};

class QDisplay_MainWindow : public QDisplay_Base {

  // Window variables & flags
  char *m_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  char *m_negative_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  int m_width = 512;
  int m_height = 512;
  int m_steps = 70;
  int m_seed = 0;
  double m_cfg = 7.5;

  std::string m_selected_model = "";
  std::string m_ckpt_path = "";
  std::vector<listItem> m_ckpt_files;
  std::unique_ptr<Canvas> m_canvas = 0;
  bool finishedRendering = false;

  // ImGui Specific config
  const std::string c_windowName = "Text To Image";
  std::pair<int, int> m_windowSize {};
  const float c_configBarHeight = 320.0f;

public:
  // Initialise render manager references
  QDisplay_MainWindow(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;
    reloadModelFiles();
  }

  void reloadModelFiles() {
    // Load model files
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::MODELS_DIRECTORY.get())) {
        listItem i{.m_name = entry.path().filename()};
        m_ckpt_files.push_back(i);
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::MODELS_DIRECTORY, "MODELS_DIRECTORY");
    }
  }

  std::string getLatestFile() {
    std::string outfile = "";
    fs::file_time_type write_time;

    try {
      for (const auto &entry : fs::directory_iterator("data/" + CONFIG::OUTPUT_DIRECTORY.get() + "/txt2img")) {
        if (entry.is_regular_file()) {
          outfile = entry.path();
        }
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::OUTPUT_DIRECTORY, "OUTPUT_DIRECTORY");
    }

    return outfile;
  }

  void renderCanvas() {
    m_canvas.reset();
    m_canvas = std::unique_ptr<Canvas>(
        new Canvas(CONFIG::CANVAS_SIZE_X_LIMIT.get(), CONFIG::CANVAS_SIZE_Y_LIMIT.get(), "generated"));
    m_canvas->rendered = false;
    m_renderManager->textToImage(*m_canvas, m_prompt, m_negative_prompt, 1, m_steps, m_cfg, m_seed, m_width, m_height, m_canvas->rendered,
                                 m_selected_model);
  }

  void canvasWindow() {
    ImGui::BeginChild("GL");
    if (m_canvas) {

      // Generate option only available whilst a canvas isn't pending
      if (m_canvas->rendered) {
        if (ImGui::Button("Generate")) {
          renderCanvas();
        }
      } else {
        ImGui::Text("Generating image...");
      }

      // Once canvas is marked as rendered display on screen
      if (m_canvas->rendered) {
        ImGui::Text("canvas width: %d canvas height:%d", m_canvas->m_width, m_canvas->m_height);
        if (ImGui::Button("Send to Canvas")) {
          // Send to main window canvas
          m_renderManager->setCanvas(*m_canvas);
        }

        // Retrieve texture file
        if (!m_canvas->textured) {
          m_canvas->loadFromImage(getLatestFile());
          m_canvas->textured = true;
        }

        ImGui::Image((void *)(intptr_t)m_canvas->m_canvas, ImVec2(256, 256));
      }
    } else {
      if (ImGui::Button("Generate")) {
        renderCanvas();
      }
    }

    ImGui::EndChild();
  }
  
  // Prompt 
  void promptHelper() {
    ImGui::BeginChild("Prompt Helper");
    ImGui::InputTextMultiline("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::InputTextMultiline("negative prompt", m_negative_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::EndChild();
  }

  void promptConfig() {
    ImGui::BeginChild("Prompt Config");

    ImGui::SliderInt("width", &m_width, 0, CONFIG::CANVAS_SIZE_X_LIMIT.get());
    if (ImGui::BeginPopupContextItem("width"))
    {
        if (ImGui::MenuItem("Reset to default: 512"))
            m_width = 512;
        ImGui::EndPopup();
    }
    ImGui::SliderInt("height", &m_height, 0, CONFIG::CANVAS_SIZE_Y_LIMIT.get());
    if (ImGui::BeginPopupContextItem("height"))
    {
        if (ImGui::MenuItem("Reset to default: 512"))
            m_height = 512;
        ImGui::EndPopup();
    }
    
    ImGui::InputInt("steps", &m_steps);
    ImGui::InputInt("seed", &m_seed);
    ImGui::InputDouble("cfg scale", &m_cfg, 0.1);

    if (ImGui::BeginCombo("models", m_selected_model.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_ckpt_files) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_selected_model = item.m_name;
        }
        if (item.m_isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Reload Models")) {
      reloadModelFiles();
    }

    ImGui::EndChild();
  }

  void textualInversion() {
    ImGui::Begin("Textual Inversion", 0);
  }

  virtual void render() {
    getWindowSize(m_windowSize);
    ImGui::SetNextWindowPos(ImVec2(0, float(m_windowSize.second) - c_configBarHeight));
    ImGui::SetNextWindowSize(ImVec2(float(m_windowSize.first), c_configBarHeight));

    ImGui::Begin(c_windowName.c_str(), 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );

    ImGui::Columns(3);
    ImGui::SetColumnOffset(1, 420.0f);
    {
      promptHelper();
    }
    ImGui::NextColumn();
    {
      promptConfig();
    }
    ImGui::NextColumn();
    { 
      canvasWindow();
    }

    ImGui::End();
  }
};