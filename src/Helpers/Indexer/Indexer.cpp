#include "Indexer.h"
#include "Config/config.h"
#include "MetaData.h"
#include "Config/config.h"
#include "QLogger.h"
#include <thread>

Indexer::Indexer(std::string folder_path) : m_crawler(folder_path) {
  m_root_path = folder_path;
  m_cachefile = CONFIG::INDEX_CACHE.get();

  // TODO: load cached index and populate our inverted index and crawler with the last run values

  m_II = InvertedIndex();
  m_crawlerThread = std::thread(&Indexer::crawlerThreadWorker, this);

  // TODO: launch caching on seperate thread, cacheing thread allows for add / update / remove calls

  // TODO: launch save to filesystem on seperate thread, cache saved periodically
}

Indexer::~Indexer() {}

// Run crawler traversal function every X time period to identify any filesystem changes
void Indexer::crawlerThreadWorker() {
  while (m_crawl) {
    QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Indexer::crawlerThreadWorker firing");
    m_crawler.traverse();
    std::this_thread::sleep_for(c_crawlerSleepTime);
  }
}

/*
  // Pretend we load metadata
  metadata a{"test prompt blah blah", "", "12345", "UNIPC", "512", "512"};
  metadata b{"another test", "bad", "54321", "UNIPC", "512", "768"};

  meta_node fileA{"a.png", "a"};
  meta_node fileB{"b.png", "b"};

  m_II.add(a, fileA);
  m_II.add(b, fileB);

  std::vector<meta_node> results = m_II.search("test 512 768");

  for (auto &i : results) {
    std::cout << "Found match: " << i.m_filepath << std::endl;
  }
*/