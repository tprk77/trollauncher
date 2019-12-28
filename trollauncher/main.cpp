// Copyright (c) 2019 Tim Perkins

// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the “Software”), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <cstdlib>
#include <iostream>

#include "trollauncher/java_detector.hpp"
#include "trollauncher/modpack_installer.hpp"

int main(const int argc, const char** argv)
{
  using namespace tl;
  std::srand(std::time(nullptr));
  if (argc != 2) {
    std::cerr << "Usage: trollauncher MODPACK-PATH" << std::endl;
    return 1;
  }
  const std::string modpack_path = argv[1];
  std::error_code ec;
  auto mi_ptr = ModpackInstaller::Create(modpack_path, &ec);
  if (mi_ptr == nullptr) {
    std::cerr << "ERROR: " << ec << " " << ec.message() << std::endl;
    return 1;
  }
  if (!mi_ptr->Install(&ec)) {
    std::cerr << "ERROR: " << ec << " " << ec.message() << std::endl;
    return 1;
  }
  std::cout << "Modpack installed successfully!" << std::endl;
  return 0;
}
