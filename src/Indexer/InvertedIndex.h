// Creates an inverted index hash map of the different "keys" that can point to a given filepath
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#include <thread>

#include "Config/config.h"
#include "Indexer/MetaData.h"

class InvertedIndex {

private:
  // Inverted hashmap, one string key can point to many different related nodes
  std::map<std::string, std::vector<meta_node>> m_index;

  std::set<std::string> getUniqueTerms(std::string str) {
    // Strip special characters
    std::string clean_str;
    for (char c : str) {
      if (std::isalpha(c) || std::isdigit(c) || std::isspace(c) ||
          c == '.') {   // Check if character is alphabetic, numeral, full stop or a space
        clean_str += c; // If so, add it to clean_str
      }
    }

    // Convert to lowercase
    std::transform(clean_str.begin(), clean_str.end(), clean_str.begin(), ::tolower);

    // Split into unique terms
    std::set<std::string> key_set;
    if (clean_str.find(' ') == std::string::npos) {
      key_set.insert(clean_str);
    } else {
      std::stringstream ss(clean_str);
      std::string word;
      while (std::getline(ss, word, ' ')) {
        key_set.insert(word);
      }
    }
    return key_set;
  }

  // This function takes two vectors and returns a new vector containing only the elements that are present in both
  std::set<meta_node> intersection(const std::set<meta_node> &v1, const std::set<meta_node> &v2) {
    std::set<meta_node> result;
    std::set<meta_node> s1(v1.begin(), v1.end());
    for (auto &node : v2) {
      if (s1.find(node) != s1.end()) {
        result.insert(node);
      }
    }
    return result;
  }

public:
  InvertedIndex() {}
  ~InvertedIndex() {}

  void add(metadata data, meta_node node) {

    // Check on each unique key
    for (auto &key : getUniqueTerms(data.getKeys())) {

      // Append to existing key
      if (m_index.find(key) != m_index.end()) {
        for (auto &nodes : m_index[key]) {
          // Already existing reference for this file
          if (nodes.m_filepath == node.m_filepath) {
            return;
          }
        }
        m_index[key].emplace_back(node);
      } else {
        // Brand new entry
        std::vector<meta_node> newNode{node};
        m_index[key] = newNode;
      }
    }
  }
  void remove();

  // This search should check that the found nodes exist for each term,
  // Only return the nodes that have matched each and every term
  std::set<meta_node> search(const std::string &searchTerms) {
    std::map<std::string, std::set<meta_node>> searchResults;

    std::set<std::string> uniques = getUniqueTerms(searchTerms);

    // Loop through search terms
    for (std::string term : uniques) {
      QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Stripped search term: ", term);
      // Our search should be lower case
      std::transform(term.begin(), term.end(), term.begin(), ::tolower);

      // No entry for search term, bad search
      if (m_index.find(term) == m_index.end()) {
        return std::set<meta_node>{};
      } else {
        std::vector<meta_node> nodes = m_index[term];

        // Only keep nodes that match every existing term
        std::set<meta_node> matchedNodes;
        for (auto &node : nodes) {
          bool matched = true;
          for (auto &existingTerm : searchResults) {
            if (std::find(existingTerm.second.begin(), existingTerm.second.end(), node) == existingTerm.second.end()) {
              matched = false;
              break;
            }
          }
          if (matched) {
            matchedNodes.insert(node);
          }
        }
        searchResults[term] = matchedNodes;
      }
    }

    // Combine all matched nodes for each search term
    std::set<meta_node> finalNodes;
    if (uniques.size() > 0) {
      finalNodes = searchResults[*uniques.begin()];
      for (auto &term : uniques) {
        std::set<meta_node> nodes = searchResults[term];
        finalNodes = intersection(finalNodes, nodes);
      }
    }
    return finalNodes;
  }
};