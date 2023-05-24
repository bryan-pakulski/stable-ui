#pragma once

#include "QLogger.h"
#include <imgui.h>
#include <memory>
#include "Config/config.h"
#include "Config/types.h"

struct Base_Error {
  std::string m_errorMessage;
  std::string m_id;
  bool alive = true;
  bool active = false;

  virtual void action() = 0;
  virtual ~Base_Error(){};
};

struct Error : public Base_Error {
  Error(const std::string &error) {
    m_errorMessage = error;
    m_id = "ERROR_" + m_errorMessage;
  }

  ~Error() {}

  void action() {
    if (active == false) {
      ImGui::OpenPopup(m_id.c_str());
      active = true;
    }

    if (ImGui::BeginPopupModal(m_id.c_str())) {
      ImGui::Text("%s", m_errorMessage.c_str());

      if (ImGui::Button("OK")) {
        alive = false;
      }
      ImGui::EndPopup();
    }
  }
};

template <class T> struct ConfigError : public Base_Error {
  T *m_variable;
  std::string m_configPath;

  ConfigError(T &variable, std::string &configPath) : m_variable(&variable), m_configPath(configPath) {
    m_id = "CONFIG_ERROR_" + m_configPath;
  };

  ~ConfigError() {}

  void setConfigValue() {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "ErrorHandler::setConfigValue Setting additional configuration for",
                               m_configPath, "to value", m_variable->get());
    CONFIG::setConfig(*m_variable, m_configPath);
  }

  void action() {
    if (active == false) {
      ImGui::OpenPopup(m_id.c_str());
      active = true;
    }

    if (ImGui::BeginPopupModal(m_id.c_str())) {
      ImGui::Text("Enter a value for configuration:");

      // Attempt different dynamic casts to determine input type from imgui
      if (dynamic_cast<CString *>(m_variable)) {
        ImGui::InputText(m_configPath.c_str(), m_variable->getC_str(),
                         10); // TODO: use CONFIG::DEFAULT_BUFFER_LENGTH.get() when string
                              // length issue is fixed, see `types.h`
      } else if (dynamic_cast<CInt *>(m_variable)) {
        ImGui::InputInt(m_configPath.c_str(), dynamic_cast<CInt *>(m_variable)->ref());
      } else if (dynamic_cast<CFloat *>(m_variable)) {
        ImGui::InputFloat(m_configPath.c_str(), dynamic_cast<CFloat *>(m_variable)->ref());
      }

      if (ImGui::Button("OK")) {
        setConfigValue();
        alive = false;
      }
      ImGui::EndPopup();
    }
  }
};

class ErrorHandler {
public:
  static ErrorHandler &GetInstance() {
    static ErrorHandler handle;
    return handle;
  }

  /*
   * Set error state to true and store message, log an error in the QLogger
   *
   * @param const string
   */
  void setError(const std::string &error) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, error);
    m_errors.emplace_back(std::unique_ptr<Base_Error>(new Error(error)));
  }

  /*
   * Set error state to true and store message, log an error in the QLogger
   *
   * @param ref T
   * @param const string
   */
  template <class T> void setConfigError(T &variable, std::string configPath) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Config error, prompting for config:", configPath);
    m_errors.emplace_back(std::unique_ptr<ConfigError<T>>(new ConfigError<T>(variable, configPath)));
  }

  /*
   * Polls for new errors and render where necessary
   */
  void pollErrors() {
    std::vector<std::unique_ptr<Base_Error>>::iterator it = m_errors.begin();

    while (it != m_errors.end()) {
      if (it->get()->alive) {
        it->get()->action();
        ++it;
      } else {
        it = m_errors.erase(it);
      }
    }
  }

  /*
   * Returns error status
   *
   * @return bool
   */
  bool hasError() const { return m_errors.size() > 0; }

private:
  std::vector<std::unique_ptr<Base_Error>> m_errors;

  ErrorHandler() {}
  ~ErrorHandler() {}
};
