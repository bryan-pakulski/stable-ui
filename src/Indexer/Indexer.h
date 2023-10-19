#pragma once

#include <chrono>
#include <string>
#include <map>
#include <thread>

#include "Helpers/Timer.h"
#include "Helpers/asyncQueue.h"
#include "Indexer/InvertedIndex.h"
#include "Indexer/Crawler.h"
#include "Indexer/MetaData.h"

class Indexer {
public:
public:
  Indexer(std::string folder_path);
  ~Indexer();

  // Return all nodes that matches the searchterm
  std::set<meta_node> find(const std::string &searchTerm);

  std::vector<std::string> forceUpdate(bool collectLatestFiles = false) {
    QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Indexer::forceUpdate firing");
    m_crawler.traverse(collectLatestFiles);
    return m_crawler.getLatestCrawledFiles();
  }

private:
  std::string m_root_path;
  std::string m_cachefile;

  std::thread m_crawlerThread;
  std::thread m_queueThread;

  std::mutex m_queueMutex;
  std::condition_variable m_queueConditionVariable;

  std::shared_ptr<asyncQueue<std::pair<std::string, QUEUE_STATUS>>> m_crawlerQueue;
  bool m_crawl = true;
  timer_killer m_kill_timer = timer_killer();

  // Stores all our XMP data in memory for fast lookup
  InvertedIndex m_II;

  // Crawl filesystem recursively for select filetypes to feed our inverted index
  Crawler m_crawler;

private:
  // Load and save data for our inverted index in memory
  void saveIndex();
  void loadIndex();

  // Thread worker functions
  void crawlerThreadWorker();
  void indexerCrawlerQueueThreadWorker();
};