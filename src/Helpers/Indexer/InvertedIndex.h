// Creates an inverted index hash map of the different "keys" that can point to a given filepath
#include <map>
#include <string>
#include <vector>
#include <thread>

#include "Config/config.h"

struct meta_node {
  std::string m_filepath;
  std::string m_filehash;

  bool operator==(const meta_node &other) const { return (m_filepath == other.m_filepath); }
  bool operator!=(const meta_node &other) const { return (m_filepath != other.m_filepath); }
  bool operator<(const meta_node &other) const { return m_filepath < other.m_filepath; }
  bool operator>(const meta_node &other) const { return m_filepath > other.m_filepath; }
};

struct metadata {
  std::string prompt;
  std::string negative_prompt;
  std::string model_hash;
  std::string sampler;
  std::string width;
  std::string height;

  // Return all metadata as a concatenated string, these form the basis of our inverted index search
  std::string getKeys() {
    return prompt + " " + negative_prompt + " " + model_hash + " " + sampler + " " + width + " " + height;
  }
};

class InvertedIndex {

private:
  // Inverted hashmap, one string key can point to many different related nodes
  std::map<std::string, std::vector<meta_node>> m_index;

  std::set<std::string> getUniqueTerms(const std::string &str) {
    std::set<std::string> key_set; // Use set to store unique keys
    std::stringstream ss(str);
    std::string word;
    while (std::getline(ss, word, ' ')) {
      key_set.insert(word); // Insert into set to avoid duplicates
    }
    return key_set;
  }

  // This function takes two vectors and returns a new vector containing only the elements that are present in both
  std::vector<meta_node> intersection(const std::vector<meta_node> &v1, const std::vector<meta_node> &v2) {
    std::vector<meta_node> result;
    std::set<meta_node> s1(v1.begin(), v1.end());
    for (auto &node : v2) {
      if (s1.find(node) != s1.end()) {
        result.push_back(node);
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
          if (nodes.m_filehash == node.m_filehash) {
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
  std::vector<meta_node> search(const std::string &searchTerms) {
    std::map<std::string, std::vector<meta_node>> searchResults;

    std::set<std::string> uniques = getUniqueTerms(searchTerms);

    // Loop through search terms
    for (auto &term : uniques) {

      // No entry for search term, bad search
      if (m_index.find(term) == m_index.end()) {
        return std::vector<meta_node>{};
      } else {
        std::vector<meta_node> nodes = m_index[term];

        // Only keep nodes that match every existing term
        std::vector<meta_node> matchedNodes;
        for (auto &node : nodes) {
          bool matched = true;
          for (auto &existingTerm : searchResults) {
            if (std::find(existingTerm.second.begin(), existingTerm.second.end(), node) == existingTerm.second.end()) {
              matched = false;
              break;
            }
          }
          if (matched) {
            matchedNodes.push_back(node);
          }
        }
        searchResults[term] = matchedNodes;
      }
    }

    // Combine all matched nodes for each search term
    std::vector<meta_node> finalNodes;
    if (uniques.size() > 0) {
      finalNodes = searchResults[*uniques.begin()];
      for (auto &term : uniques) {
        std::vector<meta_node> nodes = searchResults[term];
        finalNodes = intersection(finalNodes, nodes);
      }
    }
    return finalNodes;
  }
};