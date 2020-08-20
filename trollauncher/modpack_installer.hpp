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
#include <functional>
#include <memory>
#include <optional>
#include <system_error>

#include "trollauncher/profile_data.hpp"

namespace tl {

using ProgressFunc = std::function<void(std::size_t, const std::string&)>;

std::vector<ProfileData> GetInstalledProfiles(std::error_code* ec);
std::vector<ProfileData> GetInstalledProfiles(const std::filesystem::path& dot_minecraft_path,
                                              std::error_code* ec);

class ModpackInstaller final {
 public:
  using Ptr = std::shared_ptr<ModpackInstaller>;

  static Ptr Create(const std::filesystem::path& modpack_path, std::error_code* ec);
  static Ptr Create(const std::filesystem::path& modpack_path,
                    const std::filesystem::path& dot_minecraft_path, std::error_code* ec);

  std::string GetUniqueProfileName() const;
  std::string GetRandomProfileIcon() const;

  bool PrepInstaller(std::error_code* ec);
  std::optional<bool> IsForgeInstalled();

  bool Install(const std::string& profile_name, const std::string& profile_icon,
               std::error_code* ec, const ProgressFunc& progress_func = nullptr);
  bool Install(const std::string& profile_id, const std::string& profile_name,
               const std::string& profile_icon, const std::filesystem::path& install_path,
               std::error_code* ec, const ProgressFunc& progress_func = nullptr);

 private:
  ModpackInstaller();

  struct Data_;
  std::unique_ptr<Data_> data_;
};

class ModpackUpdater final {
 public:
  using Ptr = std::shared_ptr<ModpackUpdater>;

  static Ptr Create(const std::string profile_id, const std::filesystem::path& modpack_path,
                    std::error_code* ec);
  static Ptr Create(const std::string profile_id, const std::filesystem::path& modpack_path,
                    const std::filesystem::path& dot_minecraft_path, std::error_code* ec);

  bool PrepInstaller(std::error_code* ec);
  std::optional<bool> IsForgeInstalled();

  bool Update(std::error_code* ec, const ProgressFunc& progress_func = nullptr);

 private:
  ModpackUpdater();

  struct Data_;
  std::unique_ptr<Data_> data_;
};

}  // namespace tl

#endif  // TROLLAUNCHER_MODPACK_INSTALLER_HPP_
