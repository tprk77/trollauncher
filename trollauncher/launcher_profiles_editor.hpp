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

#ifndef TROLLAUNCHER_LAUCHER_PROFILES_EDITOR_HPP_
#define TROLLAUNCHER_LAUCHER_PROFILES_EDITOR_HPP_

#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>

namespace tl {

class LauncherProfilesEditor final {
 public:
  using Ptr = std::shared_ptr<LauncherProfilesEditor>;

  static Ptr Create(const std::filesystem::path& launcher_profiles_path, std::error_code* ec);

  bool Refresh(std::error_code* ec);

  bool HasProfileWithId(const std::string& id) const;
  bool HasProfileWithName(const std::string& name) const;

  std::string GetNewUniqueId() const;
  std::string GetNewUniqueName() const;

  bool PatchForgeProfile(std::error_code* ec);
  bool WriteProfile(const std::string& id, const std::string& name, const std::string& icon,
                    const std::string& version, const std::filesystem::path& game_path,
                    const std::optional<std::filesystem::path>& java_path_opt, std::error_code* ec);

 private:
  LauncherProfilesEditor();

  struct Data_;
  std::unique_ptr<Data_> data_;
};

}  // namespace tl

#endif  // TROLLAUNCHER_LAUCHER_PROFILES_EDITOR_HPP_
