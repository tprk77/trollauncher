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

#include "trollauncher/modpack_installer.hpp"

#include <fstream>
#include <numeric>
#include <optional>

#include <libzippp.h>
#include <nlohmann/json.hpp>

#include "trollauncher/error_codes.hpp"
#include "trollauncher/forge_installer.hpp"
#include "trollauncher/java_detector.hpp"
#include "trollauncher/launcher_profiles_editor.hpp"
#include "trollauncher/utils.hpp"

#ifndef ITS_A_UNIX_SYSTEM
#ifndef _WIN32
#define ITS_A_UNIX_SYSTEM true
#else
#define ITS_A_UNIX_SYSTEM false
#endif
#endif

namespace tl {

namespace {

namespace fs = std::filesystem;
namespace nl = nlohmann;
namespace zpp = libzippp;

std::optional<fs::path> GetDefaultDotMinecraftPath();
fs::path GetDefaultInstallPath(const fs::path& dot_minecraft_path, const std::string& name);
std::optional<fs::path> GetTopLevelDirectory(zpp::ZipArchive* zip_ptr);
bool ExtractAll(const fs::path& install_path, zpp::ZipArchive* zip_ptr,
                const std::optional<fs::path>& strip_prefix_opt, std::error_code* ec);

}  // namespace

struct ModpackInstaller::Data_ {
  fs::path modpack_path;
  fs::path dot_minecraft_path;
  LauncherProfilesEditor::Ptr lpe_ptr;
  std::string id;
  std::string name;
  std::string icon;
  fs::path install_path;
  std::unique_ptr<zpp::ZipArchive> zip_ptr;
};

ModpackInstaller::ModpackInstaller() : data_(std::make_unique<ModpackInstaller::Data_>())
{
  // Do nothing
}

ModpackInstaller::Ptr ModpackInstaller::Create(const std::filesystem::path& modpack_path,
                                               std::error_code* ec)
{
  std::optional<fs::path> dot_minecraft_path_opt = GetDefaultDotMinecraftPath();
  if (!dot_minecraft_path_opt) {
    SetError(ec, Error::DOT_MINECRAFT_NO_DEFAULT);
    return nullptr;
  }
  return Create(modpack_path, dot_minecraft_path_opt.value(), ec);
}

ModpackInstaller::Ptr ModpackInstaller::Create(const std::filesystem::path& modpack_path,
                                               const std::filesystem::path& dot_minecraft_path,
                                               std::error_code* ec)
{
  if (!fs::exists(modpack_path)) {
    SetError(ec, Error::MODPACK_NONEXISTENT);
    return nullptr;
  }
  // TODO Test if this is a zip file
  if (!fs::is_regular_file(modpack_path)) {
    SetError(ec, Error::MODPACK_NOT_REGULAR_FILE);
    return nullptr;
  }
  auto zip_ptr = std::make_unique<zpp::ZipArchive>(modpack_path.string());
  if (!zip_ptr->open(zpp::ZipArchive::READ_ONLY)) {
    SetError(ec, Error::MODPACK_ZIP_OPEN_FAILED);
    return nullptr;
  }
  const fs::path launcher_profiles_path = dot_minecraft_path / "launcher_profiles.json";
  auto lpe_ptr = LauncherProfilesEditor::Create(launcher_profiles_path, ec);
  if (lpe_ptr == nullptr) {
    return nullptr;
  }
  auto mi_ptr = Ptr(new ModpackInstaller());
  mi_ptr->data_->modpack_path = modpack_path;
  mi_ptr->data_->dot_minecraft_path = dot_minecraft_path;
  mi_ptr->data_->lpe_ptr = std::move(lpe_ptr);
  mi_ptr->data_->id = mi_ptr->data_->lpe_ptr->GetNewUniqueId();
  mi_ptr->data_->name = mi_ptr->data_->lpe_ptr->GetNewUniqueName();
  mi_ptr->data_->icon = "TNT";
  mi_ptr->data_->install_path = GetDefaultInstallPath(dot_minecraft_path, mi_ptr->data_->id);
  mi_ptr->data_->zip_ptr = std::move(zip_ptr);
  return mi_ptr;
}

std::string ModpackInstaller::GetName() const
{
  return data_->name;
}

void ModpackInstaller::SetName(const std::string& name)
{
  data_->name = name;
}

std::string ModpackInstaller::GetIcon() const
{
  return data_->icon;
}

void ModpackInstaller::SetIcon(const std::string& icon)
{
  data_->icon = icon;
}

std::filesystem::path ModpackInstaller::GetInstallPath() const
{
  return data_->install_path;
}

void ModpackInstaller::SetInstallPath(const std::filesystem::path& install_path)
{
  data_->install_path = install_path;
}

bool ModpackInstaller::Install(std::error_code* ec)
{
  std::error_code fs_ec;
  if (!fs::exists(data_->install_path)) {
    fs::create_directories(data_->install_path, fs_ec);
    if (fs_ec) {
      SetError(ec, Error::MODPACK_DESTINATION_CREATION_FAILED);
      return false;
    }
  }
  if (!fs::is_directory(data_->install_path)) {
    SetError(ec, Error::MODPACK_DESTINATION_NOT_DIRECTORY);
    return false;
  }
  const auto modpack_dir_iter = fs::directory_iterator(data_->install_path);
  if (begin(modpack_dir_iter) != end(modpack_dir_iter)) {
    SetError(ec, Error::MODPACK_DESTINATION_NOT_EMPTY);
    return false;
  }
  const std::optional<fs::path> tl_dir_opt = GetTopLevelDirectory(data_->zip_ptr.get());
  if (!ExtractAll(data_->install_path, data_->zip_ptr.get(), tl_dir_opt, ec)) {
    return false;
  }
  // TODO The forge install should probably be optional? But then where to get the version?
  const fs::path forge_installer_path = data_->install_path / "trollauncher" / "installer.jar";
  auto fi_ptr = ForgeInstaller::Create(forge_installer_path, data_->dot_minecraft_path, ec);
  if (fi_ptr == nullptr) {
    return false;
  }
  if (!fi_ptr->IsInstalled()) {
    if (!fi_ptr->Install(ec)) {
      return false;
    }
    if (!data_->lpe_ptr->PatchForgeProfile(ec)) {
      return false;
    }
  }
  const std::optional<fs::path> java_path_opt = JavaDetector::GetJavaVersion8();
  if (!data_->lpe_ptr->WriteProfile(data_->id, data_->name, data_->icon, fi_ptr->GetForgeVersion(),
                                    data_->install_path, java_path_opt, ec)) {
    return false;
  }
  return true;
}

namespace {

std::optional<fs::path> GetDefaultDotMinecraftPath()
{
  // Look for the default location: "${HOME}/.minecraft" or "%APPDATA%\.minecraft"
  const std::string default_home_name = (ITS_A_UNIX_SYSTEM ? "HOME" : "APPDATA");
  const std::optional<std::string> home_opt = GetEnvironmentVar(default_home_name);
  if (!home_opt) {
    return std::nullopt;
  }
  fs::path dot_minecraft_path = fs::path(home_opt.value()) / ".minecraft";
  if (!fs::exists(dot_minecraft_path) || !fs::is_directory(dot_minecraft_path)) {
    return std::nullopt;
  }
  return dot_minecraft_path;
}

fs::path GetDefaultInstallPath(const fs::path& dot_minecraft_path, const std::string& id)
{
  return dot_minecraft_path / "trollauncher" / id;
}

std::optional<fs::path> GetTopLevelDirectory(zpp::ZipArchive* zip_ptr)
{
  const zpp::ZipEntry first_entry = zip_ptr->getEntry(0);
  if (first_entry.isNull()) {
    return std::nullopt;
  }
  const fs::path entry_path = first_entry.getName();
  if (!entry_path.has_parent_path()) {
    return std::nullopt;
  }
  const fs::path maybe_tl_dir = *entry_path.begin();
  if (maybe_tl_dir == "mods" || maybe_tl_dir == "config" || maybe_tl_dir == "trollauncher") {
    return std::nullopt;
  }
  for (std::size_t ii = 1; ii < static_cast<std::size_t>(zip_ptr->getEntriesCount()); ++ii) {
    const zpp::ZipEntry other_entry = zip_ptr->getEntry(ii);
    if (!other_entry.isFile()) {
      continue;
    }
    const fs::path other_entry_path = other_entry.getName();
    if (!other_entry_path.has_parent_path() || *other_entry_path.begin() != maybe_tl_dir) {
      return std::nullopt;
    }
  }
  return maybe_tl_dir;
}

fs::path StripPrefix(const fs::path& orig_path, const fs::path& prefix_path)
{
  const std::size_t orig_length = std::distance(orig_path.begin(), orig_path.end());
  const std::size_t prefix_length = std::distance(prefix_path.begin(), prefix_path.end());
  if (orig_length < prefix_length) {
    return orig_path;
  }
  const fs::path orig_prefix_path =
      std::accumulate(orig_path.begin(), std::next(orig_path.begin(), prefix_length), fs::path(),
                      [](const auto& p, const auto& r) { return p / r; });
  if (orig_prefix_path != prefix_path) {
    return orig_path;
  }
  const fs::path orig_remainder_path =
      std::accumulate(std::next(orig_path.begin(), prefix_length), orig_path.end(), fs::path(),
                      [](const auto& p, const auto& r) { return p / r; });
  return orig_remainder_path;
}

bool ExtractAll(const fs::path& install_path, zpp::ZipArchive* zip_ptr,
                const std::optional<fs::path>& strip_prefix_opt, std::error_code* ec)
{
  std::error_code fs_ec;
  const std::vector<zpp::ZipEntry> zip_entries = zip_ptr->getEntries();
  for (const auto& zip_entry : zip_entries) {
    if (!zip_entry.isFile()) {
      continue;
    }
    const fs::path entry_path = zip_entry.getName();
    const fs::path dest_path =
        (strip_prefix_opt ? install_path / StripPrefix(entry_path, strip_prefix_opt.value())
                          : install_path / entry_path);
    // Create the parent directory if we need to
    const fs::path dest_parent_path = dest_path.parent_path();
    if (!fs::exists(dest_parent_path)) {
      fs::create_directories(dest_parent_path, fs_ec);
      if (fs_ec) {
        SetError(ec, Error::MODPACK_UNZIP_FAILED);
        return false;
      }
    }
    // Create the actual file
    const std::ios_base::openmode ofs_flags =
        std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
    std::ofstream file_ofs(dest_path, ofs_flags);
    if (!file_ofs.good()) {
      SetError(ec, Error::MODPACK_UNZIP_FAILED);
      return false;
    }
    const int zip_error_code = zip_ptr->readEntry(zip_entry, file_ofs);
    if (zip_error_code != LIBZIPPP_OK) {
      SetError(ec, Error::MODPACK_UNZIP_FAILED);
      return false;
    }
  }
  return true;
}

}  // namespace

}  // namespace tl
