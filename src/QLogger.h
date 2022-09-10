#pragma once

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#define QLOGGER_LOGFILE "QLog.txt"

enum class LOGLEVEL { INFO, WARN, ERR };

class QLogger {

public:
  clock_t LAST_WRITE_TIME{};
  std::thread myThread;

  static QLogger &GetInstance() {
    static QLogger logger;
    return logger;
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
    } catch (const std::exception &e) {
      QLogger::GetInstance().Error(std::string("Caught Exception: "), e.what());
    }
  }

  template <typename T, typename... Args>
  void Log(LOGLEVEL logLevel, T message, Args... args) {
    if (logLevel == LOGLEVEL::INFO) {
      log << std::string(getDateTime()).append(" - [INFO]: ") << message
          << ", ";
      Info(args...);
    } else if (logLevel == LOGLEVEL::WARN) {
      log << std::string(getDateTime()).append(" - [WARN]: ") << message
          << ", ";
      Warning(args...);
    } else if (logLevel == LOGLEVEL::ERR) {
      log << std::string(getDateTime()).append(" - [ERR]: ") << message << ", ";
      Error(args...);
    }

    updateLogTimestamp();
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

  // Open file on instantiation
  QLogger() {
    log.open(QLOGGER_LOGFILE, std::ios_base::app);
    myThread = std::thread(&QLogger::myThread, this);
  }

  ~QLogger() {
    log.close();
    myThread.join();
  }

  void updateLogTimestamp() {
    if (stat(QLOGGER_LOGFILE, &logStat) == 0) {
      LAST_WRITE_TIME = clock();
    }
  }

  template <typename T> void Error(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Error(T message, Args... args) {
    log << message << ", ";
    Error(args...);
  }

  template <typename T> void Warning(T message) { log << message << std::endl; }
  template <typename T, typename... Args>
  void Warning(T message, Args... args) {
    log << message << ", ";
    Warning(args...);
  }

  template <typename T> void Info(T message) { log << message << std::endl; }
  template <typename T, typename... Args> void Info(T message, Args... args) {
    log << message << ", ";
    Info(args...);
  }

  // Empty EOL for single message logs without ...args
  void Error() { log << std::endl; }
  void Warning() { log << std::endl; }
  void Info() { log << std::endl; }

  /*
   * Gets current date time, strips new lines and returns string
   *
   * @return string
   */
  static std::string getDateTime() {
    time_t _tm = time(nullptr);
    struct tm *curtime = localtime(&_tm);
    std::string strTime = std::string(asctime(curtime));
    strTime.erase(std::remove(strTime.begin(), strTime.end(), '\n'),
                  strTime.end());

    return strTime;
  }
};