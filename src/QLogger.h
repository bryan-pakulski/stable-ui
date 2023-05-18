#pragma once

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <mutex>

#include "Config/config.h"

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#define QLOGGER_LOGFILE "data/logs/sd_server.log"

enum class LOGLEVEL { INFO, WARN, ERR, DEBUG };

class QLogger {

public:
  clock_t m_LAST_WRITE_TIME{};
  std::thread m_Thread;

  static QLogger &GetInstance() {
    static QLogger s_Logger;
    return s_Logger;
  }

  /*
   * Handle a generic captured exception (C++11) using the ... operator
   * Exception is rethrown and error is logged
   *
   * @param ePtr
   */
  static void ExceptionHandler(std::exception_ptr ePtr) {
    try {
      if (ePtr) {
        std::rethrow_exception(ePtr);
      }
    } catch (const std::exception &err) {
      QLogger::GetInstance().Error(std::string("Caught Exception: "), err.what());
    }
  }

  template <typename T, typename... Args> void Log(LOGLEVEL logLevel, T message, Args... args) {
    m_Thread = std::thread(&QLogger::_Log<T, Args...>, this, logLevel, message, args...);
    m_Thread.detach();
  }

  void resetLog() {
    log.close();
    log.open(QLOGGER_LOGFILE, std::ios::out | std::ios::trunc);
    updateLogTimestamp();
  }

  // Prohibit external replication constructs
  QLogger(QLogger const &) = delete;

  // Prohibit external assignment operations
  void operator=(QLogger const &) = delete;

private:
  // Output file
  std::ofstream log;
  struct stat logStat {};
  std::mutex m_mutex;

  // Open file on instantiation
  QLogger() { log.open(QLOGGER_LOGFILE, std::ios_base::app); }

  ~QLogger() { log.close(); }

  void updateLogTimestamp() {
    if (stat(QLOGGER_LOGFILE, &logStat) == 0) {
      m_LAST_WRITE_TIME = clock();
    }
  }

  template <typename T, typename... Args> void _Log(LOGLEVEL logLevel, T message, Args... args) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (logLevel == LOGLEVEL::INFO) {
      log << std::string(getDateTime()).append(" - [info]: ") << message << ", ";
      Info(args...);
    } else if (logLevel == LOGLEVEL::WARN) {
      log << std::string(getDateTime()).append(" - [warn]: ") << message << ", ";
      Warning(args...);
    } else if (logLevel == LOGLEVEL::ERR) {
      log << std::string(getDateTime()).append(" - [err]: ") << message << ", ";
      Error(args...);
    } else if (logLevel == LOGLEVEL::DEBUG && CONFIG::ENABLE_DEBUG_LOGGING.get() == 1) {
      log << std::string(getDateTime()).append(" - [dbg]: ") << message << ", ";
      Debug(args...);
    }

    updateLogTimestamp();
  }

  template <typename T> void Error(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Error(T message, Args... args) {
    log << message << ", ";
    Error(args...);
  }

  template <typename T> void Warning(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Warning(T message, Args... args) {
    log << message << ", ";
    Warning(args...);
  }

  template <typename T> void Info(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Info(T message, Args... args) {
    log << message << ", ";
    Info(args...);
  }

  template <typename T> void Debug(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Debug(T message, Args... args) {
    log << message << ", ";
    Debug(args...);
  }

  // Empty EOL for single message logs without ...args
  void Error() { log << std::endl; }
  void Warning() { log << std::endl; }
  void Info() { log << std::endl; }
  void Debug() { log << std::endl; }

  /*
   * Gets current date time, strips new lines and returns string
   *
   * @return string
   */
  static std::string getDateTime() {
    time_t _tm = time(nullptr);
    struct tm *curtime = localtime(&_tm);
    std::string strTime = std::string(asctime(curtime));
    strTime.erase(std::remove(strTime.begin(), strTime.end(), '\n'), strTime.end());

    return strTime;
  }
};