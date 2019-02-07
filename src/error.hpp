#pragma once

#include <iostream>

namespace yslang {
static void error(const std::string &msg) {
  std::cerr << "err: " << msg << std::endl;
  exit(1);
}
} // namespace yslang
