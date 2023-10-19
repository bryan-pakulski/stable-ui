#pragma once

#include "Helpers/QLogger.h"
#include <imgui.h>
#include <memory>
#include "Config/config.h"
#include "Config/types.h"

struct Base_Error {
  std::string m_title;
  std::string m_errorMessage;
  std::string m_id;
  bool alive = true;
  bool active = false;

  virtual void action() = 0;
  virtual ~Base_Error(){};
};

struct Error : public Base_Error {
  Error(std::string title, const std::string &error) {
    m_title = title;
    m_errorMessage = error;
    m_id = "ERROR_" + m_errorMessage;
  }

  ~Error() {}

  void action() {
    ImGui::SetNextWindowSize(ImVec2(480.0f, 260.0f));
    if (active == false) {
      ImGui::OpenPopup(m_title.c_str());
      active = true;
    }

    if (ImGui::BeginPopupModal(m_title.c_str(), 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
      ImGui::TextWrapped("%s", m_errorMessage.c_str());

      auto windowWidth = ImGui::GetWindowSize().x;
      ImGui::SetCursorPosX(((windowWidth)*0.5f) - (150.0f / 2.0f));
      if (ImGui::Button("Okay", ImVec2(150, 40))) {
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
  void setError(std::string title, const std::string &error) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, error);
    m_errors.emplace_back(std::unique_ptr<Base_Error>(new Error(title, error)));
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
