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

#ifndef TROLLAUNCHER_MODPACK_INSTALLER_HPP_
#define TROLLAUNCHER_MODPACK_INSTALLER_HPP_

#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>

namespace tl {

class ModpackInstaller final {
 public:
  using Ptr = std::shared_ptr<ModpackInstaller>;

  static Ptr Create(const std::filesystem::path& modpack_path, std::error_code* ec);
  static Ptr Create(const std::filesystem::path& modpack_path,
                    const std::filesystem::path& dot_minecraft_path, std::error_code* ec);

  std::string GetName() const;
  void SetName(const std::string& name);

  std::string GetIcon() const;
  void SetIcon(const std::string& icon);

  std::filesystem::path GetInstallPath() const;
  void SetInstallPath(const std::filesystem::path& install_path);

  bool PrepInstall(std::error_code* ec);
  std::optional<bool> IsForgeInstalled();

  bool Install(std::error_code* ec);

 private:
  ModpackInstaller();

  struct Data_;
  std::unique_ptr<Data_> data_;
};

}  // namespace tl

#endif  // TROLLAUNCHER_MODPACK_INSTALLER_HPP_
