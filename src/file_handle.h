#ifndef BUCKET_FILE_HANDLE_H
#define BUCKET_FILE_HANDLE_H
#include <stdio.h>

class FileHandle {
public:
  FileHandle(const char *name, const char *mode);
  FileHandle(FILE *file);
  ~FileHandle() noexcept;
  FILE *getFile();

private:
  FILE *mFile;
};

#endif // BUCKET_FILE_HANDLE_H