#include "../QLogger.h"
#include "../ThirdParty/sha1/sha1.h"

inline std::string getFileHash(const char *fileName) {
  SHA1 sha1stream;

  FILE *f = fopen(fileName, "rb");
  int bytes;
  unsigned char data[1024];
  while ((bytes = fread(data, 1, 1024, f)) != 0) {
    sha1stream.add(data, 1024);
  }

  if (f == NULL) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "SHA1::getFileHash Unable to sha1 file: ", fileName);
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "SHA1::getFileHash Computing file hash for: ", fileName);
  }

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SHA1::getFileHash Computed Hash: ", sha1stream.getHash());
  return sha1stream.getHash();
}