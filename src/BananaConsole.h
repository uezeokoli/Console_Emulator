#pragma once

#include <string>

#include "BananaCpu.h"
#include "BananaGpu.h"
#include "BananaMemory.h"

class BananaConsole {
private:
  std::string filename_;
  size_t file_size_;

public:
  BananaMemory memory;
  BananaGpu gpu;
  uint8_t input = 0;
  BananaConsole(const std::string &filename);
  void run();
};
