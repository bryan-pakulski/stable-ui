#pragma once

#include "Config/config.h"
#include "Config/types.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/GLHelper.h"
#include "Helpers/QLogger.h"

class YamlDisplayBuilder : public QDisplay_Base {

public:
  bool m_terminate = false;

  std::vector<YAML::Node> m_windows;
  std::map<std::string, BaseType> m_variables;

public:
  // Initialise render manager reference
  YamlDisplayBuilder(std::string yamlPath, std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    static YAML::Node guiConfig = YAML::LoadFile(yamlPath);
    YAML::Node windows = guiConfig["windows"];

    // Load each window into memory
    for (YAML::const_iterator it = windows.begin(); it != windows.end(); ++it) {
      m_windows.push_back(windows[it->first.as<std::string>()]);
    }
  }

  ~YamlDisplayBuilder() {}

  virtual void render() {

    YAML::Node window = m_windows[m_activeWindow];

    ImGui::Begin(window["name"].as<std::string>().c_str());

    for (YAML::const_iterator it = window.begin(); it != window.end(); ++it) {
      std::string type = it->first.as<std::string>();

      // Read types and construct gui elements

      // TODO: Read variables?

      /*
        IMGUI TEXT INPUT
        PARAMS:
          varName (required)
          inputType (required, string / int / float / boolean)
      */
      if (type == "input") {
      }

      /*
        IMGUI BUTTON
        PARAMS:
          label (required)
          action (required, close or command)
          command (required for command action, runs a script on docker container)
      */
      if (type == "button") {
        if (ImGui::Button(it->second["label"].as<std::string>().c_str())) {

          if (it->second["action"].as<std::string>() == "command") {
            // TODO: run action command
          } else if (it->second["action"].as<std::string>() == "close") {
            m_terminate = true;
          }
        }
      }
    }

    ImGui::End();
  }

private:
  // Default active window index
  int m_activeWindow = 0;
};