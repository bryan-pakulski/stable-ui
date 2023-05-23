#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "QLogger.h"

// Files to actually index
static const std::set<std::string> s_filetypes = {".png", ".jpg", ".jpeg"};

struct FileInfo {
  std::filesystem::file_time_type last_modified;
  size_t file_size;
};

class Crawler {
public:
  Crawler(const std::string &path) : m_rootPath(path) {
    QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Crawler::Crawler Initialising on path:", m_rootPath);
  }
  ~Crawler() {}

  // Crawl filesystem
  void traverse() {
    // First check our existing entries and account for deletions and updates
    updateIndex();

    for (const auto &entry : std::filesystem::recursive_directory_iterator(m_rootPath)) {
      const std::string filepath = entry.path().string();
      // Check if this is a new entry, only allow files from our fileTypes vector
      if (entry.is_regular_file() && !m_fileInfo.contains(filepath)) {
        if (s_filetypes.contains(entry.path().extension())) {
          QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Crawler::Traverse Found new file:", filepath);
          m_fileInfo[filepath] =
              FileInfo{std::filesystem::last_write_time(filepath), std::filesystem::file_size(filepath)};
        }
      }
    }
  }

private:
  std::string m_rootPath;
  std::unordered_map<std::string, FileInfo> m_fileInfo;

  bool is_file_modified(const std::string &filepath) {
    std::filesystem::file_time_type last_modified = std::filesystem::last_write_time(filepath);
    size_t file_size = std::filesystem::file_size(filepath);

    // Compare current file info with stored file info
    FileInfo &info = m_fileInfo[filepath];
    if (info.last_modified != last_modified || info.file_size != file_size) {
      return true;
    } else {
      return false;
    }
  }

  // Iterate over our cralwed files and update on any changes
  void updateIndex() {
    for (auto it = m_fileInfo.begin(); it != m_fileInfo.end();) {
      if (!std::filesystem::exists(it->first)) {
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Crawler::updateIndex Removing deleted file:", it->first);
        m_fileInfo.erase(it->first);
      } else if (is_file_modified(it->first)) {
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Crawler::updateIndex Updating modified file:", it->first);
        m_fileInfo[it->first] =
            FileInfo{std::filesystem::last_write_time(it->first), std::filesystem::file_size(it->first)};
      } else {
        ++it;
      }
    }
  }
};