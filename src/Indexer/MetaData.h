#pragma once

#if _WIN32
#define WIN_ENV
#else
#define UNIX_ENV
#endif

#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

// #define ENABLE_XMP_CPP_INTERFACE 1

// Must be defined to instantiate template classes
#define TXMP_STRING_TYPE std::string

// Must be defined to give access to XMPFiles
#define XMP_INCLUDE_XMPFILES 1

// Ensure XMP templates are instantiated
#include "XMP.incl_cpp"

// Provide access to the API
#include "XMP.hpp"

#include <iostream>
#include <fstream>

#include "QLogger.h"

struct meta_node {
  std::string m_filepath;

  bool operator==(const meta_node &other) const { return (m_filepath == other.m_filepath); }
  bool operator!=(const meta_node &other) const { return (m_filepath != other.m_filepath); }
  bool operator<(const meta_node &other) const { return m_filepath < other.m_filepath; }
  bool operator>(const meta_node &other) const { return m_filepath > other.m_filepath; }
};

struct metadata {
  std::map<std::string, std::string> m_map{{"stableui-prompt", ""},          {"stableui-negative_prompt", ""},
                                           {"stableui-negative_prompt", ""}, {"stableui-model_hash", ""},
                                           {"stableui-sampler", ""},         {"stableui-width", ""},
                                           {"stableui-height", ""}};

  // Return all metadata as a concatenated string, these form the basis of our inverted index search
  std::string getKeys() {
    std::ostringstream oss;
    for (auto &kv : m_map) {
      oss << kv.second << " ";
    }
    std::string result = oss.str();
    result.pop_back(); // remove last space
    return result;
  }
};

class XMP {
public:
  static XMP &GetInstance() {
    static XMP s_xmp;
    return s_xmp;
  }

  // Prohibit external replication constructs
  XMP(XMP const &) = delete;

  // Prohibit external assignment operations
  void operator=(XMP const &) = delete;

  /**
   *	Initializes the toolkit and attempts to open a file for reading metadata.  Initially
   * an attempt to open the file is done with a handler, if this fails then the file is opened with
   * packet scanning. Once the file is open several properties are read and displayed in the console.
   * The XMP object is then dumped to a text file and the resource file is closed.
   */
  std::pair<meta_node, metadata> readFile(std::string filename, bool dumpToFile = false) {
    meta_node node{filename};
    metadata nodeMetadata{};

    if (!m_initialised) {
      return std::pair<meta_node, metadata>{node, nodeMetadata};
    }

    try {
      // Options to open the file with - read only and use a file handler
      XMP_OptionBits opts = kXMPFiles_OpenForRead | kXMPFiles_OpenUseSmartHandler;

      bool ok;
      SXMPFiles myFile;
      std::string status = "";

      // First we try and open the file
      ok = myFile.OpenFile(filename, kXMP_UnknownFile, opts);
      if (!ok) {
        status += "No smart handler available for " + filename + "\n";
        status += "Trying packet scanning.\n";

        // Now try using packet scanning
        opts = kXMPFiles_OpenForUpdate | kXMPFiles_OpenUsePacketScanning;
        ok = myFile.OpenFile(filename, kXMP_UnknownFile, opts);
      }

      // If the file is open then read the metadata
      if (ok) {
        QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "XMP::readFile", status, filename, " is opened  successfully");

        // Create the xmp object and get the xmp data
        SXMPMeta meta;
        myFile.GetXMP(&meta);

        // Extract XMP values
        std::string simpleVal;
        bool exists;
        for (auto &k : nodeMetadata.m_map) {
          exists = meta.GetProperty(kXMP_NS_DC, k.first.c_str(), &simpleVal, NULL);
          if (exists) {
            nodeMetadata.m_map[k.first.c_str()] = simpleVal;
          } else {
            simpleVal.clear();
          }
        }

        // Dump the current xmp object to a file
        if (dumpToFile) {
          std::ofstream dumpFile;
          std::string dumpPath = filename + "-XMPDump.txt";
          dumpFile.open(dumpPath, std::ios::out);
          meta.DumpObject(DumpXMPToFile, &dumpFile);
          dumpFile.close();
          QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "XMP::readFile dumped file to: ", dumpPath);
        }

        // Close the SXMPFile.  The resource file is already closed if it was
        // opened as read only but this call must still be made.
        myFile.CloseFile();
      } else {
        QLogger::GetInstance().Log(LOGLEVEL::ERR, "XMP::readFile unable to open: ", filename);
      }
    } catch (XMP_Error &e) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "XMP::readFile", e.GetErrMsg());
    }

    return std::pair<meta_node, metadata>{node, nodeMetadata};
  }

private:
  bool m_initialised = true;

  XMP() { initialise(); }
  ~XMP() {
    // Terminate the toolkit
    SXMPFiles::Terminate();
    SXMPMeta::Terminate();
  }

  /**
   * Client defined callback function to dump XMP to a file.  In this case an output file stream is used
   * to write a buffer, of length bufferSize, to a text file.  This callback is called multiple
   * times during the DumpObject() operation.  See the XMP API reference for details of
   * XMP_TextOutputProc() callbacks.
   */
  static XMP_Status DumpXMPToFile(void *refCon, XMP_StringPtr buffer, XMP_StringLen bufferSize) {
    XMP_Status status = 0;

    try {
      std::ofstream *outFile = static_cast<std::ofstream *>(refCon);
      (*outFile).write(buffer, bufferSize);
    } catch (XMP_Error &e) {
      std::cout << e.GetErrMsg() << std::endl;
      return -1; // Return a bad status
    }

    return status;
  }

  void initialise() {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "XMP::initialise Initialising XMP Toolkit");
    if (!SXMPMeta::Initialize()) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "XMP::initialise couldn't initialise toolkit");
      m_initialised = false;
    }
    XMP_OptionBits options = 0;
#ifdef UNIX_ENV
    options |= kXMPFiles_ServerMode;
#endif
    // Must initialize SXMPFiles before we use it
    if (!SXMPFiles::Initialize(options)) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "XMP::initialise Could not initialize SXMPFiles");
      m_initialised = false;
    }
  }
};