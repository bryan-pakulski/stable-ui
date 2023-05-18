#pragma once

#include <imgui.h>
#include <filesystem>

#include "../../../Display/ErrorHandler.h"
#include "../../QDisplay_Base.h"
#include "../../YamlDisplayBuilder.h"
#include <imgui_stdlib.h>

#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

class QDisplay_PluginsWindow : public QDisplay_Base {

private:
  void loadPluginList() {}
  std::vector<listItem> m_ModulesList;
  std::vector<std::unique_ptr<YamlDisplayBuilder>> m_VisibleModules;

public:
  QDisplay_PluginsWindow(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    // Load modules list from yaml
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODULES_CONFIGURATION_FILE.get());
      YAML::Node modules = configFile["modules"];
      for (YAML::const_iterator it = modules.begin(); it != modules.end(); ++it) {
        // Check that the module contains gui configuration
        listItem i{.m_name = it->first.as<std::string>(), .m_key = it->second["gui-path"].as<std::string>()};
        if (i.m_key != "") {
          m_ModulesList.push_back(i);
        }
      }
    } catch (const YAML::Exception &) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR,
                                 "QDisplay_PluginsWindow::QDisplay_PluginsWindow Failed to parse yaml file: ",
                                 CONFIG::MODULES_CONFIGURATION_FILE.get());
      return;
    }
  }

  virtual void render() {
    std::vector<std::unique_ptr<YamlDisplayBuilder>>::iterator iter;

    // Iterate and delete any modules that have closed, update iterator to not break our look
    for (iter = m_VisibleModules.begin(); iter != m_VisibleModules.end();) {
      if (iter->get()->m_terminate)
        iter = m_VisibleModules.erase(iter);
      else {
        iter->get()->render();
        ++iter;
      }
    }
  }

  // When a new menu is selected add to display builder
  void menus() {
    for (auto &item : m_ModulesList) {
      if (ImGui::MenuItem(item.m_name.c_str())) {
        // TODO: only push back if doesn't already exist
        m_VisibleModules.push_back(
            std::unique_ptr<YamlDisplayBuilder>(new YamlDisplayBuilder(item.m_key, m_stableManager, m_window)));
      }
    }
  }
};