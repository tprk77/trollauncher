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
#include <iostream>
#include <numeric>

#include <libzippp.h>
#include <nlohmann/json.hpp>

#include "trollauncher/error_codes.hpp"
#include "trollauncher/forge_installer.hpp"
#include "trollauncher/java_detector.hpp"
#include "trollauncher/keeplist_processor.hpp"
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
bool ProfileLooksLikeAnInstall(const ProfileData& profile_data);
bool ProfileLooksLikeAnInstall(const fs::path& profile_path);
std::optional<std::vector<fs::path>> GetDirFilePaths(const fs::path& dir_path);
std::optional<fs::path> GetTopLevelDirectory(zpp::ZipArchive* zip_ptr);
fs::path StripPrefix(const fs::path& orig_path, const fs::path& prefix_path);
bool ExtractOne(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                const std::optional<fs::path>& add_prefix_opt, const fs::path& entry_path);
bool ExtractAll(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                const std::optional<fs::path>& strip_prefix_opt);
bool ExtractOverwrites(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                       const std::optional<fs::path>& strip_prefix_opt,
                       const KeeplistProcessor::Ptr& klp_ptr);
fs::path GetBackupZipPath(const fs::path& dot_minecraft_path, const std::string& id);
bool CreateBackupZipFile(const fs::path& backup_path, const fs::path& profile_path,
                         const std::vector<fs::path>& overwrite_paths);

}  // namespace

std::vector<ProfileData> GetInstalledProfiles(std::error_code* ec)
{
  std::optional<fs::path> dot_minecraft_path_opt = GetDefaultDotMinecraftPath();
  if (!dot_minecraft_path_opt) {
    SetError(ec, Error::DOT_MINECRAFT_NO_DEFAULT);
    return {};
  }
  return GetInstalledProfiles(dot_minecraft_path_opt.value(), ec);
}

std::vector<ProfileData> GetInstalledProfiles(const fs::path& dot_minecraft_path,
                                              std::error_code* ec)
{
  const fs::path launcher_profiles_path = dot_minecraft_path / "launcher_profiles.json";
  auto lpe_ptr = LauncherProfilesEditor::Create(launcher_profiles_path, ec);
  if (lpe_ptr == nullptr) {
    return {};
  }
  std::vector<ProfileData> installed_profiles;
  for (const ProfileData& profile_data : lpe_ptr->GetProfiles()) {
    if (!ProfileLooksLikeAnInstall(profile_data)) {
      continue;
    }
    installed_profiles.push_back(profile_data);
  }
  return installed_profiles;
}

struct ModpackInstaller::Data_ {
  fs::path modpack_path;
  fs::path dot_minecraft_path;
  LauncherProfilesEditor::Ptr lpe_ptr;
  std::unique_ptr<zpp::ZipArchive> zip_ptr;
  bool is_prepped;
  ForgeInstaller::Ptr fi_ptr;
};

ModpackInstaller::ModpackInstaller() : data_(std::make_unique<ModpackInstaller::Data_>())
{
  // Do nothing
}

ModpackInstaller::Ptr ModpackInstaller::Create(const fs::path& modpack_path, std::error_code* ec)
{
  std::optional<fs::path> dot_minecraft_path_opt = GetDefaultDotMinecraftPath();
  if (!dot_minecraft_path_opt) {
    SetError(ec, Error::DOT_MINECRAFT_NO_DEFAULT);
    return nullptr;
  }
  return Create(modpack_path, dot_minecraft_path_opt.value(), ec);
}

ModpackInstaller::Ptr ModpackInstaller::Create(const fs::path& modpack_path,
                                               const fs::path& dot_minecraft_path,
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
  mi_ptr->data_->zip_ptr = std::move(zip_ptr);
  mi_ptr->data_->is_prepped = false;
  mi_ptr->data_->fi_ptr = nullptr;
  return mi_ptr;
}

std::string ModpackInstaller::GetUniqueProfileName() const
{
  return data_->lpe_ptr->GetNewUniqueName();
}

std::string ModpackInstaller::GetRandomProfileIcon() const
{
  return GetRandomIcon();
}

bool ModpackInstaller::PrepInstaller(std::error_code* ec)
{
  std::optional<fs::path> temp_path_opt = CreateTempDir();
  if (!temp_path_opt) {
    SetError(ec, Error::MODPACK_PREP_INSTALL_TEMPDIR_FAILED);
    return false;
  }
  const fs::path& temp_path = temp_path_opt.value();
  const std::optional<fs::path> tl_dir_opt = GetTopLevelDirectory(data_->zip_ptr.get());
  if (!ExtractOne(data_->zip_ptr.get(), temp_path, tl_dir_opt, "trollauncher/installer.jar")) {
    SetError(ec, Error::MODPACK_PREP_INSTALL_UNZIP_FAILED);
    return false;
  }
  const fs::path forge_installer_path = temp_path / "trollauncher" / "installer.jar";
  auto fi_ptr = ForgeInstaller::Create(forge_installer_path, data_->dot_minecraft_path, ec);
  if (fi_ptr == nullptr) {
    return false;
  }
  data_->fi_ptr = std::move(fi_ptr);
  data_->is_prepped = true;
  return true;
}

std::optional<bool> ModpackInstaller::IsForgeInstalled()
{
  if (!data_->is_prepped) {
    return std::nullopt;
  }
  return data_->fi_ptr->IsInstalled();
}

bool ModpackInstaller::Install(const std::string& profile_name, const std::string& profile_icon,
                               std::error_code* ec)
{
  const std::string profile_id = data_->lpe_ptr->GetNewUniqueId();
  const fs::path install_path = GetDefaultInstallPath(data_->dot_minecraft_path, profile_id);
  return Install(profile_id, profile_name, profile_icon, install_path, ec);
}

bool ModpackInstaller::Install(const std::string& profile_id, const std::string& profile_name,
                               const std::string& profile_icon, const fs::path& install_path,
                               std::error_code* ec)
{
  std::error_code fs_ec;
  if (!fs::exists(install_path)) {
    fs::create_directories(install_path, fs_ec);
    if (fs_ec) {
      SetError(ec, Error::MODPACK_DESTINATION_CREATION_FAILED);
      return false;
    }
  }
  if (!fs::is_directory(install_path)) {
    SetError(ec, Error::MODPACK_DESTINATION_NOT_DIRECTORY);
    return false;
  }
  const auto modpack_dir_iter = fs::directory_iterator(install_path);
  if (begin(modpack_dir_iter) != end(modpack_dir_iter)) {
    SetError(ec, Error::MODPACK_DESTINATION_NOT_EMPTY);
    return false;
  }
  // Step 0: Prep install
  if (!data_->is_prepped && !PrepInstaller(ec)) {
    return false;
  }
  // Step 1: Install Forge
  if (!data_->fi_ptr->IsInstalled()) {
    if (!data_->fi_ptr->Install(ec)) {
      return false;
    }
    if (!data_->lpe_ptr->PatchForgeProfile(ec)) {
      return false;
    }
  }
  // Step 2: Extract modpack
  const std::optional<fs::path> tl_dir_opt = GetTopLevelDirectory(data_->zip_ptr.get());
  if (!ExtractAll(data_->zip_ptr.get(), install_path, tl_dir_opt)) {
    SetError(ec, Error::MODPACK_UNZIP_FAILED);
    return false;
  }
  // Step 3: Write profile
  const std::string forge_version = data_->fi_ptr->GetForgeVersion();
  const std::optional<fs::path> java_path_opt = JavaDetector::GetJavaVersion8();
  if (!data_->lpe_ptr->WriteProfile(profile_id, profile_name, profile_icon, forge_version,
                                    install_path, java_path_opt, ec)) {
    return false;
  }
  return true;
}

struct ModpackUpdater::Data_ {
  std::string profile_id;
  fs::path modpack_path;
  fs::path dot_minecraft_path;
  LauncherProfilesEditor::Ptr lpe_ptr;
  std::unique_ptr<zpp::ZipArchive> zip_ptr;
  bool is_prepped;
  ForgeInstaller::Ptr fi_ptr;
};

ModpackUpdater::ModpackUpdater() : data_(std::make_unique<ModpackUpdater::Data_>())
{
  // Do nothing
}

ModpackUpdater::Ptr ModpackUpdater::Create(const std::string profile_id,
                                           const fs::path& modpack_path, std::error_code* ec)
{
  std::optional<fs::path> dot_minecraft_path_opt = GetDefaultDotMinecraftPath();
  if (!dot_minecraft_path_opt) {
    SetError(ec, Error::DOT_MINECRAFT_NO_DEFAULT);
    return nullptr;
  }
  return Create(profile_id, modpack_path, dot_minecraft_path_opt.value(), ec);
}

ModpackUpdater::Ptr ModpackUpdater::Create(const std::string profile_id,
                                           const fs::path& modpack_path,
                                           const fs::path& dot_minecraft_path, std::error_code* ec)
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
  auto mu_ptr = Ptr(new ModpackUpdater());
  mu_ptr->data_->profile_id = profile_id;
  mu_ptr->data_->modpack_path = modpack_path;
  mu_ptr->data_->dot_minecraft_path = dot_minecraft_path;
  mu_ptr->data_->lpe_ptr = std::move(lpe_ptr);
  mu_ptr->data_->zip_ptr = std::move(zip_ptr);
  mu_ptr->data_->is_prepped = false;
  mu_ptr->data_->fi_ptr = nullptr;
  return mu_ptr;
}

bool ModpackUpdater::PrepInstaller(std::error_code* ec)
{
  std::optional<fs::path> temp_path_opt = CreateTempDir();
  if (!temp_path_opt) {
    SetError(ec, Error::MODPACK_PREP_INSTALL_TEMPDIR_FAILED);
    return false;
  }
  const fs::path& temp_path = temp_path_opt.value();
  const std::optional<fs::path> tl_dir_opt = GetTopLevelDirectory(data_->zip_ptr.get());
  if (!ExtractOne(data_->zip_ptr.get(), temp_path, tl_dir_opt, "trollauncher/installer.jar")) {
    SetError(ec, Error::MODPACK_PREP_INSTALL_UNZIP_FAILED);
    return false;
  }
  const fs::path forge_installer_path = temp_path / "trollauncher" / "installer.jar";
  auto fi_ptr = ForgeInstaller::Create(forge_installer_path, data_->dot_minecraft_path, ec);
  if (fi_ptr == nullptr) {
    return false;
  }
  data_->fi_ptr = std::move(fi_ptr);
  data_->is_prepped = true;
  return true;
}

std::optional<bool> ModpackUpdater::IsForgeInstalled()
{
  if (!data_->is_prepped) {
    return std::nullopt;
  }
  return data_->fi_ptr->IsInstalled();
}

bool ModpackUpdater::Update(std::error_code* ec)
{
  std::error_code fs_ec;
  if (!data_->lpe_ptr->Refresh(ec)) {
    return false;
  }
  const std::optional<ProfileData> profile_data_opt = data_->lpe_ptr->GetProfile(data_->profile_id);
  if (!profile_data_opt) {
    SetError(ec, Error::PROFILE_NONEXISTENT);
    return false;
  }
  const ProfileData& profile_data = profile_data_opt.value();
  if (!ProfileLooksLikeAnInstall(profile_data) || !profile_data.game_path_opt) {
    SetError(ec, Error::PROFILE_NOT_AN_INSTALL);
    return false;
  }
  const fs::path profile_path = profile_data.game_path_opt.value();
  if (!fs::is_directory(profile_path)) {
    SetError(ec, Error::MODPACK_DESTINATION_NOT_DIRECTORY);
    return false;
  }
  // Step 0: Prep install
  if (!data_->is_prepped && !PrepInstaller(ec)) {
    return false;
  }
  // Step 1: Get existing files not in the keeplist
  const auto klp_ptr = KeeplistProcessor::CreateDefault();
  if (klp_ptr == nullptr) {
    SetError(ec, Error::MODPACK_KEEPLIST_FAILED);
    return false;
  }
  const std::optional<std::vector<fs::path>> all_file_paths_opt = GetDirFilePaths(profile_path);
  if (!all_file_paths_opt) {
    SetError(ec, Error::PROFILE_GET_FILES_FAILED);
    return false;
  }
  const std::vector<fs::path> overwrite_paths =
      klp_ptr->FilterOverwritePaths(all_file_paths_opt.value());
  // Step 2: Create zip file of all old files
  const fs::path backup_path = GetBackupZipPath(data_->dot_minecraft_path, data_->profile_id);
  if (!CreateBackupZipFile(backup_path, profile_path, overwrite_paths)) {
    SetError(ec, Error::PROFILE_BACKUP_FAILED);
    return false;
  }
  // Step 3: Delete all old files
  for (const fs::path& overwrite_path : overwrite_paths) {
    const fs::path full_path = profile_path / overwrite_path;
    fs::remove(full_path, fs_ec);
  }
  // Step 4: Extract new files not in the keeplist
  const std::optional<fs::path> tl_dir_opt = GetTopLevelDirectory(data_->zip_ptr.get());
  if (!ExtractOverwrites(data_->zip_ptr.get(), profile_path, tl_dir_opt, klp_ptr)) {
    SetError(ec, Error::MODPACK_UNZIP_FAILED);
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

bool ProfileLooksLikeAnInstall(const ProfileData& profile_data)
{
  if (!profile_data.type_opt || profile_data.type_opt.value() != "custom") {
    return false;
  }
  if (!profile_data.game_path_opt
      || !ProfileLooksLikeAnInstall(profile_data.game_path_opt.value())) {
    return false;
  }
  return true;
}

bool ProfileLooksLikeAnInstall(const fs::path& profile_path)
{
  // Check for the presence "trollauncher/install.jar"
  fs::path possible_installer_path = profile_path / "trollauncher" / "installer.jar";
  const bool is_trollauncher_like = fs::is_regular_file(possible_installer_path);
  // Check for the absence of "assets", "libraries", and "versions"
  fs::path possible_assets_path = profile_path / "assets";
  fs::path possible_libraries_path = profile_path / "libraries";
  fs::path possible_versions_path = profile_path / "versions";
  const bool is_minecraft_like =
      (fs::is_directory(possible_assets_path) && fs::is_directory(possible_libraries_path)
       && fs::is_directory(possible_versions_path));
  return is_trollauncher_like && !is_minecraft_like;
}

std::optional<std::vector<fs::path>> GetDirFilePaths(const fs::path& dir_path)
{
  std::error_code fs_ec;
  std::vector<fs::path> raw_file_paths;
  const auto update_dir_iter = fs::recursive_directory_iterator(dir_path, fs_ec);
  if (fs_ec) {
    return std::nullopt;
  }
  for (const fs::path& path : update_dir_iter) {
    if (fs::is_regular_file(path)) {
      raw_file_paths.push_back(path);
    }
  }
  std::vector<fs::path> relative_file_paths;
  for (const fs::path& path : raw_file_paths) {
    relative_file_paths.push_back(StripPrefix(path, dir_path));
  }
  return relative_file_paths;
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

bool ExtractOne(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                const std::optional<fs::path>& add_prefix_opt, const fs::path& entry_path)
{
  std::error_code fs_ec;
  const fs::path complete_path =
      (add_prefix_opt ? add_prefix_opt.value() / entry_path : entry_path);
  zpp::ZipEntry zip_entry = zip_ptr->getEntry(complete_path.generic_string());
  if (!zip_entry.isFile()) {
    return false;
  }
  const fs::path dest_path = extract_path / entry_path;
  // Create the parent directory if we need to
  const fs::path dest_parent_path = dest_path.parent_path();
  if (!fs::exists(dest_parent_path)) {
    fs::create_directories(dest_parent_path, fs_ec);
    if (fs_ec) {
      return false;
    }
  }
  // Create the actual file
  const std::ios_base::openmode ofs_flags =
      std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
  std::ofstream file_ofs(dest_path, ofs_flags);
  if (!file_ofs.good()) {
    return false;
  }
  const int zip_error_code = zip_ptr->readEntry(zip_entry, file_ofs);
  if (zip_error_code != LIBZIPPP_OK) {
    return false;
  }
  return true;
}

bool ExtractAll(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                const std::optional<fs::path>& strip_prefix_opt)
{
  return ExtractOverwrites(zip_ptr, extract_path, strip_prefix_opt, nullptr);
}

bool ExtractOverwrites(const zpp::ZipArchive* zip_ptr, const fs::path& extract_path,
                       const std::optional<fs::path>& strip_prefix_opt,
                       const KeeplistProcessor::Ptr& klp_ptr)
{
  std::error_code fs_ec;
  const std::vector<zpp::ZipEntry> zip_entries = zip_ptr->getEntries();
  for (const auto& zip_entry : zip_entries) {
    if (!zip_entry.isFile()) {
      continue;
    }
    const fs::path entry_path = zip_entry.getName();
    const fs::path stripped_entry_path =
        (strip_prefix_opt ? StripPrefix(entry_path, strip_prefix_opt.value()) : entry_path);
    if (klp_ptr != nullptr && !klp_ptr->IsOverwritePath(stripped_entry_path)) {
      continue;
    }
    const fs::path dest_path = extract_path / stripped_entry_path;
    // Create the parent directory if we need to
    const fs::path dest_parent_path = dest_path.parent_path();
    if (!fs::exists(dest_parent_path)) {
      fs::create_directories(dest_parent_path, fs_ec);
      if (fs_ec) {
        return false;
      }
    }
    // Create the actual file
    const std::ios_base::openmode ofs_flags =
        std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
    std::ofstream file_ofs(dest_path, ofs_flags);
    if (!file_ofs.good()) {
      return false;
    }
    const int zip_error_code = zip_ptr->readEntry(zip_entry, file_ofs);
    if (zip_error_code != LIBZIPPP_OK) {
      return false;
    }
  }
  return true;
}

fs::path GetBackupZipPath(const fs::path& dot_minecraft_path, const std::string& id)
{
  const std::string time_str = ([]() {
    const auto now_chrono = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now_chrono);
    char time_c_str[sizeof "00000000_000000"];
    std::strftime(time_c_str, sizeof time_c_str, "%Y%m%d_%H%M%S", std::gmtime(&now_time_t));
    return std::string(time_c_str);
  }());
  return dot_minecraft_path / "trollauncher" / "backups" / id / (time_str + ".zip");
}

bool CreateBackupZipFile(const fs::path& backup_path, const fs::path& profile_path,
                         const std::vector<fs::path>& overwrite_paths)
{
  std::error_code fs_ec;
  if (fs::exists(backup_path)) {
    return false;
  }
  const fs::path backup_parent_path = backup_path.parent_path();
  if (!fs::exists(backup_parent_path)) {
    fs::create_directories(backup_parent_path, fs_ec);
    if (fs_ec) {
      return false;
    }
  }
  auto zip_ptr = std::make_unique<zpp::ZipArchive>(backup_path.string());
  if (!zip_ptr->open(zpp::ZipArchive::NEW)) {
    return false;
  }
  for (const fs::path& overwrite_path : overwrite_paths) {
    const fs::path full_path = profile_path / overwrite_path;
    if (!zip_ptr->addFile(overwrite_path.generic_string(), full_path.string())) {
      zip_ptr->close();
      fs::remove(backup_path, fs_ec);
      return false;
    }
  }
  return true;
}

}  // namespace

}  // namespace tl
