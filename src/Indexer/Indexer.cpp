#include "Indexer.h"
#include "Indexer/Crawler.h"
#include "asyncQueue.h"
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

  m_crawlerThread = std::thread(&Indexer::crawlerThreadWorker, this);
  m_queueThread = std::thread(&Indexer::indexerCrawlerQueueThreadWorker, this);

  // TODO: launch caching on seperate thread, cacheing thread allows for add / update / remove calls
  // TODO: launch save to filesystem on seperate thread, cache saved periodically
}

Indexer::~Indexer() {
  // Kill crawler sleeping thread
  m_kill_timer.kill();
  // Kill queue watching thread
  m_crawlerQueue->kill();
  m_queueConditionVariable.notify_all();

  m_crawlerThread.join();
  m_queueThread.join();
}

// Run crawler traversal function every X time period to identify any filesystem changes
void Indexer::crawlerThreadWorker() {
  while (m_kill_timer.wait_for(std::chrono::milliseconds(c_crawlerSleepTime))) {
    QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Indexer::crawlerThreadWorker firing");
    m_crawler.traverse();
  }
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
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Indexer::indexerCrawlerQueueThreadWorker Getting XMP data for",
                                   queueEntry.first);
        std::pair<meta_node, metadata> data = XMP::GetInstance().readFile(queueEntry.first);
        m_II.add(data.second, data.first);
      } else if (queueEntry.second == QUEUE_STATUS::UPDATED) {
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Indexer::indexerCrawlerQueueThreadWorker Updating XMP data for",
                                   queueEntry.first);
        // TODO: implement updating existing index entries
      } else if (queueEntry.second == QUEUE_STATUS::DELETED) {
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Indexer::indexerCrawlerQueueThreadWorker Removing XMP data for",
                                   queueEntry.first);
        // TODO: implement deleting index entries
      }
    }
  }
}

std::set<meta_node> Indexer::find(const std::string &searchTerm) {
  std::set<meta_node> results = m_II.search(searchTerm);

  if (CONFIG::ENABLE_DEBUG_LOGGING.get() == 1) {
    for (auto &i : results) {
      std::cerr << "Search match: " << i.m_filepath << std::endl;
    }
  }
  return results;
}