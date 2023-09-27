#include "file_handle.h"

FileHandle::FileHandle(const char *name, const char *mode) { this->mFile = fopen(name, mode); }
FileHandle::FileHandle(FILE *file) { this->mFile = file; }
FileHandle::~FileHandle() {
  if (this->mFile != nullptr) {
    printf("closing the file handle!\n");
    fclose(this->mFile);
  }
}

FILE *FileHandle::getFile() { return this->mFile; }
