// Copyright (c) 2020 Tim Perkins

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

#ifndef TROLLAUNCHER_KEEPLIST_PROCESSOR_HPP_
#define TROLLAUNCHER_KEEPLIST_PROCESSOR_HPP_

#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>

namespace tl {

/**
 * Paths given to the keeplist processor should be relative to the project root directory.
 * Otherwise, the keeplist my not be able to match paths.
 *
 * For example, these would be acceptable:
 *
 *     config/my-mod.toml
 *     mods/my-mod-1.14.4-0.jar
 *     trollauncher/installer.jar
 */
class KeeplistProcessor final {
 public:
  using Ptr = std::shared_ptr<KeeplistProcessor>;

  static Ptr Create(const std::filesystem::path& keeplist_path, std::error_code* ec);
  static Ptr CreateDefault();

  bool IsOverwritePath(const std::filesystem::path& path) const;

  std::vector<std::filesystem::path> FilterOverwritePaths(
      const std::vector<std::filesystem::path>& paths) const;

 private:
  KeeplistProcessor();

  struct Data_;
  std::unique_ptr<Data_> data_;
};

}  // namespace tl

#endif  // TROLLAUNCHER_KEEPLIST_PROCESSOR_HPP_
