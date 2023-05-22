#pragma once

#include <string>
#include <map>
#include "InvertedIndex.h"

class Indexer {
public:
  Indexer(std::string folder_path);
  ~Indexer();

private:
  std::string m_root_path;
  std::string m_cachefile;
  InvertedIndex m_II;

  void saveIndex();
  void loadIndex();
};