#include "Indexer.h"
#include "Config/config.h"
#include "MetaData.h"
#include "Config/config.h"

Indexer::Indexer(std::string folder_path) {
  m_root_path = folder_path;
  m_cachefile = CONFIG::INDEX_CACHE.get();

  m_II = InvertedIndex();

  // Pretend we load metadata
  metadata a{"test prompt blah blah", "", "12345", "UNIPC", "512", "512"};
  metadata b{"another test", "bad", "54321", "UNIPC", "512", "768"};

  meta_node fileA{"a.png", "a"};
  meta_node fileB{"b.png", "b"};

  m_II.add(a, fileA);
  m_II.add(b, fileB);

  // TODO: multi term match is broken
  std::vector<meta_node> results = m_II.search("test bad");

  for (auto &i : results) {
    std::cout << "Found match: " << i.m_filepath << std::endl;
  }
}

Indexer::~Indexer() {}