#include "file_handle.h"

FileHandle::FileHandle(const char *name, const char *mode) : mFile(fopen(name, mode)) {}

FileHandle::FileHandle(FILE *file) : mFile(file) {}

FileHandle::~FileHandle() noexcept {
  if (mFile != nullptr) {
    fclose(mFile);
  }
}

FILE *FileHandle::getFile() { return this->mFile; }

bool FileHandle::explicitClose() {
  if (mFile != nullptr) {
    int temp_ret = fclose(mFile);
    mFile = nullptr;
    return temp_ret == 0;
  }
  return false;
}
