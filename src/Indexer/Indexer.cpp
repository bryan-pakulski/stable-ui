#include "Indexer.h"
#include "Indexer/Crawler.h"
#include "Config/config.h"
#include "Config/config.h"
#include "Helpers/QLogger.h"

#include <chrono>
#include <thread>

Indexer::Indexer(std::string folder_path) {
  m_root_path = folder_path;
  m_cachefile = CONFIG::INDEX_CACHE.get();

  // TODO: load cached index and populate our inverted index and crawler with the last run values

  m_II = InvertedIndex();
  XMP::GetInstance();

  m_crawlerQueue = std::make_shared<asyncQueue<std::pair<std::string, QUEUE_STATUS>>>();
  m_crawler = Crawler(folder_path, m_crawlerQueue);

  forceUpdate(false);
  m_queueThread = std::thread(&Indexer::indexerCrawlerQueueThreadWorker, this);

  // TODO: launch caching on seperate thread, cacheing thread allows for add / update / remove calls
  // TODO: launch save to filesystem on seperate thread, cache saved periodically
}

Indexer::~Indexer() {

  // Kill queue watching thread
  m_crawlerQueue->kill();
  m_queueConditionVariable.notify_all();

  m_queueThread.join();
}

// Run indexer queue monitor on another thread and update the inverted index whenever the crawler discovers changes to
// the filesystem
void Indexer::indexerCrawlerQueueThreadWorker() {
  std::unique_lock<std::mutex> lock(m_queueMutex);
  while (!m_crawlerQueue->killed) {
    m_queueConditionVariable.wait(lock, [this] { return !m_crawlerQueue->empty() || m_crawlerQueue->killed; });

    if (m_crawlerQueue->killed) {
      continue;
    }

    // This is locked behind a mutex and so will only pop once there is data on the queue
    std::pair<std::string, QUEUE_STATUS> queueEntry;
    if (m_crawlerQueue->try_pop(queueEntry)) {
      if (queueEntry.second == QUEUE_STATUS::ADDED) {
        QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Indexer::indexerCrawlerQueueThreadWorker Getting XMP data for",
                                   queueEntry.first);
        std::pair<meta_node, metadata> data = XMP::GetInstance().readFile(queueEntry.first);
        m_II.add(data.second, data.first);
      } else if (queueEntry.second == QUEUE_STATUS::UPDATED) {
        QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Indexer::indexerCrawlerQueueThreadWorker Updating XMP data for",
                                   queueEntry.first);
        // TODO: implement updating existing index entries
      } else if (queueEntry.second == QUEUE_STATUS::DELETED) {
        QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Indexer::indexerCrawlerQueueThreadWorker Removing XMP data for",
                                   queueEntry.first);
        // TODO: implement deleting index entries
      }
    }
  }
}

std::set<meta_node> Indexer::find(const std::string &searchTerm) {
  std::set<meta_node> results = m_II.search(searchTerm);

  for (auto &i : results) {
    QLogger::GetInstance().Log(LOGLEVEL::DBG1, "Search match: ", i.m_filepath);
  }
  return results;
}