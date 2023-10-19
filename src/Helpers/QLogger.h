#pragma once

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <mutex>

#include "Config/config.h"
#include "Helpers/asyncQueue.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#define stat _stat
#endif

enum class LOGLEVEL { DBG1, DBG2, DBG3, DBG4, TRACE, INFO, WARN, ERR };

class QLogger {

public:
  clock_t m_LAST_WRITE_TIME{};

public:
  static QLogger &GetInstance() {
    static QLogger s_Logger;
    return s_Logger;
  }

  /*
   * Construct the log as a string and push to the logging queue
   */
  template <typename T, typename... Args> void Log(LOGLEVEL logLevel, T message, Args... args) {
    unsigned long thread_id = pthread_self();

    std::thread([this, thread_id, logLevel, message, args...]() {
      std::ostringstream log_buffer;

      if (_Log(thread_id, log_buffer, logLevel, message, args...)) {
        m_loggingQueue->push(log_buffer.str());
      }
      m_queueConditionVariable.notify_all();
    }).detach();
  }

  void resetLog() {
    m_logStream.close();
    m_logStream.open(m_filepath, std::ios::out | std::ios::trunc);
    updateLogTimestamp();
  }

  // Prohibit external replication constructs
  QLogger(QLogger const &) = delete;

  // Prohibit external assignment operations
  void operator=(QLogger const &) = delete;

private:
  std::string m_filepath;
  std::ofstream m_logStream;
  struct stat m_logStat {};

  std::shared_ptr<asyncQueue<std::string>> m_loggingQueue;
  std::thread m_queueThread;
  std::mutex m_queueMutex;
  std::condition_variable m_queueConditionVariable;

private:
  QLogger() {
    m_filepath = CONFIG::LOG_FILE.get();
    m_logStream.open(m_filepath, std::ios_base::app);

    m_loggingQueue = std::make_shared<asyncQueue<std::string>>();
    m_queueThread = std::thread(&QLogger::loggingThread, this);
  }

  ~QLogger() {
    m_logStream.close();
    m_loggingQueue->kill();
    m_queueConditionVariable.notify_all();
    m_queueThread.join();
  }

  void loggingThread() {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    while (!m_loggingQueue->killed) {
      m_queueConditionVariable.wait(lock, [this] { return !m_loggingQueue->empty() || m_loggingQueue->killed; });

      if (m_loggingQueue->killed) {
        continue;
      }

      // This is locked behind a mutex and so will only pop once there is data on the queue
      std::string logMsg;
      if (m_loggingQueue->try_pop(logMsg)) {
        m_logStream << logMsg << std::flush;
        updateLogTimestamp();
      }
    }
  }

  template <typename T, typename... Args>
  bool _Log(unsigned long thread_id, std::ostringstream &ostream, LOGLEVEL logLevel, T message, Args... args) {

    if (logLevel == LOGLEVEL::INFO) {
      ostream << getDateTime() << " - [" << thread_id << "] "
              << " - [info]: " << message;
    } else if (logLevel == LOGLEVEL::WARN) {
      ostream << getDateTime() << " - [" << thread_id << "] "
              << " - [warn]: " << message;
    } else if (logLevel == LOGLEVEL::ERR) {
      ostream << getDateTime() << " - [" << thread_id << "] "
              << " - [err]: " << message;
    } else if (logLevel == LOGLEVEL::TRACE) {
      if (CONFIG::ENABLE_TRACE_LOGGING.get() == 1) {
        ostream << getDateTime() << " - [" << thread_id << "] "
                << " - [trace]: " << message;
      }
    } else if ((int)logLevel >= CONFIG::DEBUG_LEVEL.get() - 1) {
      ostream << getDateTime() << " - [" << thread_id << "] "
              << " - [dbg" << (int)logLevel + 1 << "]: " << message;
    } else {
      return false;
    }

    // Append arguments
    ([&] { ostream << ", " << args; }(), ...);
    ostream << std::endl;

    return true;
  }

  /*
   * Gets current date time, strips new lines and returns string
   *
   * @return string
   */
  std::string getDateTime() {
    time_t _tm = time(nullptr);
    struct tm *curtime = localtime(&_tm);
    std::string strTime = std::string(asctime(curtime));
    strTime.erase(std::remove(strTime.begin(), strTime.end(), '\n'), strTime.end());

    return strTime;
  }

  void updateLogTimestamp() {
    if (stat(m_filepath.c_str(), &m_logStat) == 0) {
      m_LAST_WRITE_TIME = clock();
    }
  }
};