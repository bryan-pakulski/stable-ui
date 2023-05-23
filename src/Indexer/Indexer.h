#pragma once

#include <chrono>
#include <string>
#include <map>
#include <thread>

#include "Indexer/InvertedIndex.h"
#include "Indexer/Crawler.h"
#include "Indexer/asyncQueue.h"
#include "Indexer/MetaData.h"

// Crawler worker thread will fire every minute to check filesystem for changes
static const std::chrono::duration<long long, std::milli> c_crawlerSleepTime(60000);

class Indexer {
public:
  Indexer(std::string folder_path);
  ~Indexer();

  // Return all nodes that matches the searchterm
  std::vector<meta_node> find(const std::string &searchTerm);

private:
  std::string m_root_path;
  std::string m_cachefile;

  std::thread m_crawlerThread;
  std::thread m_queueThread;

  std::shared_ptr<asyncQueue<std::pair<std::string, QUEUE_STATUS>>> m_crawlerQueue;
  XMP m_xmpManager;

  bool m_crawl = true;

  // Stores all our XMP data in memory for fast lookup
  InvertedIndex m_II;

  // Crawl filesystem recursively for select filetypes to feed our inverted index
  Crawler m_crawler;

  // Load and save data for our inverted index in memory
  void saveIndex();
  void loadIndex();

  // Thread worker functions
  void crawlerThreadWorker();
  void indexerCrawlerQueueThreadWorker();
};