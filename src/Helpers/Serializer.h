// Design created by One Lone Coder - javidx9 - https: // www.youtube.com/watch?v=jlS1Y2-yKV0

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <stack>
#include <sstream>

class datafile {
public:
  inline void setString(const std::string &sString, const size_t nItem = 0) {
    if (nItem >= m_vContent.size())
      m_vContent.resize(nItem + 1);
    m_vContent[nItem] = sString;
  }

  inline const std::string getString(const size_t nItem = 0) const {
    if (nItem >= m_vContent.size())
      return "";
    else
      return m_vContent[nItem];
  }

  // Doubles
  inline void setReal(const double d, const size_t nItem = 0) { setString(std::to_string(d), nItem); }
  inline double getReal(const size_t nItem = 0) const { return std::atof(getString(nItem).c_str()); }

  // Ints
  inline void setInt(const int32_t n, const size_t nItem = 0) { setString(std::to_string(n), nItem); }
  inline int getInt(const size_t nItem = 0) const { return std::atoi(getString(nItem).c_str()); }

  // Ints
  inline void setUInt(const uint32_t n, const size_t nItem = 0) { setString(std::to_string(n), nItem); }
  inline uint32_t getUInt(const size_t nItem = 0) const { return std::atoi(getString(nItem).c_str()); }

  // Return number of values a property consists of
  inline size_t getValueCount() const { return m_vContent.size(); }

  // Array access operator overload allows for retrieving and setting values using []
  inline datafile &operator[](const std::string &name) {
    if (m_mapObjects.count(name) == 0) {
      m_mapObjects[name] = m_vecObjects.size();
      m_vecObjects.push_back({name, datafile()});
    }

    // Node exists
    return m_vecObjects[m_mapObjects[name]].second;
  }

  // Write datafile node and all children to file
  inline static bool write(const datafile &n, const std::string &sFileName, const std::string &sIndent = "\t",
                           const char sListSep = ',') {

    std::string sSeperator = std::string(1, sListSep) + " ";
    size_t nIndentCount = 0;

    // Fully specified lambda, recursive
    std::function<void(const datafile &, std::ofstream &)> write = [&](const datafile &n, std::ofstream &file) {
      for (auto const &property : n.m_vecObjects) {

        // Lambda util function for indentation
        auto indent = [&](const std::string &sString, const size_t nCount) {
          std::string sOut;
          for (size_t n = 0; n < nCount; n++)
            sOut += sString;
          return sOut;
        };

        // Check for children
        if (property.second.m_vecObjects.empty()) {
          file << indent(sIndent, nIndentCount) << property.first << " = ";
          size_t nItems = property.second.getValueCount();

          for (size_t i = 0; i < property.second.getValueCount(); i++) {
            // If the value being written, in string form, contains the seperation character, then the value must be
            // written inside quotation marks. Note, that if the value is the last of a list of values for a property,
            // it is not suffixed with the seperator
            size_t x = property.second.getString(i).find_first_of(sListSep);
            if (x != std::string::npos) {
              file << "\"" << property.second.getString(i) << "\"" << ((nItems > 1) ? sSeperator : "");
            } else {
              file << property.second.getString(i) << ((nItems > 1) ? sSeperator : "");
            }

            nItems--;
          }

          //  Property written, move to next line
          file << "\n";
        } else {
          // Property has children, force a new line
          file << "\n" << indent(sIndent, nIndentCount) << property.first << "\n";
          // Open braces, and update indentation
          file << indent(sIndent, nIndentCount) << "{\n";
          nIndentCount++;
          // Recursively write node
          write(property.second, file);
          file << indent(sIndent, nIndentCount) << "}\n\n";
        }
      }

      // We've finished writing a node, regardless of state, our indentation must decrease, unless we're already at top
      // level
      if (nIndentCount > 0)
        nIndentCount--;
    };

    std::ofstream file(sFileName);
    if (file.is_open()) {
      write(n, file);
      return true;
    }

    return false;
  }

  inline static bool read(datafile &n, const std::string &sFileName, const char sListSep = ',') {
    std::ifstream file(sFileName);
    if (file.is_open()) {

      std::string sPropName = "";
      std::string sPropValue = "";

      std::stack<std::reference_wrapper<datafile>> stkPath;
      stkPath.push(n);

      // Read line by line and process stack
      while (!file.eof()) {
        std::string line;
        std::getline(file, line);

        // Lambda to remove whitespace from beginning and end of supplied string
        auto trim = [](std::string &s) {
          s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
          s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
        };

        trim(line);

        if (!line.empty()) {

          // If we have an assignment operator split the string into it's key value pairs
          size_t x = line.find_first_of('=');
          if (x != std::string::npos) {
            // Key
            sPropName = line.substr(0, x);
            trim(sPropName);

            // Value
            sPropValue = line.substr(x + 1, line.size());
            trim(sPropValue);

            // Iterate through string character by character and determine if we are parsing an array
            // This also accounts for cases like follows, a = 1, 2, 3, "4,5", 6
            bool bInQuotes = false;
            std::string sToken;
            size_t nTokenCount = 0;
            for (const auto c : sPropValue) {

              if (c == '\"') {
                bInQuotes = !bInQuotes;
              } else {
                if (bInQuotes) {
                  sToken.append(1, c);
                } else {
                  if (c == sListSep) {
                    trim(sToken);
                    stkPath.top().get()[sPropName].setString(sToken, nTokenCount);
                    sToken.clear();
                    nTokenCount++;
                  } else {
                    sToken.append(1, c);
                  }
                }
              }
            }

            // Any residual characters at this point just make up the final token
            if (!sToken.empty()) {
              trim(sToken);
              stkPath.top().get()[sPropName].setString(sToken, nTokenCount);
            }
          } else {
            // If we don't have an equals operation we want to check if we are entering a new node (indicated by the
            // presence of {})
            if (line[0] == '{') {
              stkPath.push(stkPath.top().get()[sPropName]);
            } else {
              if (line[0] == '}') {
                stkPath.pop();
              } else {
                sPropName = line;
              }
            }
          }
        }
      }

      file.close();
      return true;
    }

    return false;
  }

private:
  std::vector<std::string> m_vContent;

  std::vector<std::pair<std::string, datafile>> m_vecObjects;
  std::unordered_map<std::string, size_t> m_mapObjects;
};

class Serialisable {

public:
  datafile &getDF() { return df; }

private:
  datafile df;

  // This should be overloaded by super classes to make sure we capture all the data we want to store in our node
  // structure
  datafile serialise() { return datafile(); }

  bool loadFromFile(std::string file) { return datafile().read(df, file); }
};