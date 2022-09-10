#pragma once

#include "../QLogger.h"
#include <imgui.h>

class ErrorHandler {
public:
  bool err;
  std::string message;

  static ErrorHandler &GetInstance() {
    static ErrorHandler handle;
    return handle;
  }

  /*
   * Returns error status
   *
   * @return bool
   */
  bool hasError() const { return err; }

  // Error displays popup over all other windows, clears after user closes
  // dialog
  void displayError() {
    if (err) {
      ImGui::OpenPopup("ERROR");
    }

    if (ImGui::BeginPopupModal("ERROR")) {
      ImGui::Text("%s", message.c_str());

      if (ImGui::Button("OK")) {
        err = false;
      }
      ImGui::EndPopup();
    }
  }

  /*
   * Set error state to true and store message, log an error in the QLogger
   *
   * @param const string
   */
  void setError(const std::string &error) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, error);
    err = true;
    message = error;
  }

private:
  ErrorHandler() { err = false; }
  ~ErrorHandler() {}
};
