#include <bits/types/FILE.h>
#include <cstdio>
#include <openssl/sha.h>

#include "../QLogger.h"

inline std::string getFileHash(const char *fileName) {

  unsigned char result[2 * SHA_DIGEST_LENGTH];
  unsigned char hash[SHA_DIGEST_LENGTH];
  int i;
  FILE *f = fopen(fileName, "rb");
  SHA_CTX mdContent;
  int bytes;
  unsigned char data[1024];

  if (f == NULL) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Unable to sha1 file: ", fileName);
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Computing file hash for: ", fileName);
  }

  SHA1_Init(&mdContent);
  while ((bytes = fread(data, 1, 1024, f)) != 0) {

    SHA1_Update(&mdContent, data, bytes);
  }

  SHA1_Final(hash, &mdContent);
  fclose(f);

  /** if you want to see the plain text of the hash */
  for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
    sprintf((char *)&(result[i * 2]), "%02x", hash[i]);
  }

  std::string str_hash(reinterpret_cast<char *>(result));
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Computed Hash: ", str_hash);
  return str_hash;
}