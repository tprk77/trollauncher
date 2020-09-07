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

#include "trollauncher/launcher_profiles_editor.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <thread>

#include <nlohmann/json.hpp>

#include "trollauncher/error_codes.hpp"
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

std::string GetCurrentTimeAsString(
    std::optional<std::chrono::seconds> time_modifier_opt = std::nullopt);
bool IsFileWritable(const fs::path& path);
fs::path AddFilenamePrefix(const fs::path& path, const std::string& prefix);
bool WriteLauncherProfilesJson(const fs::path& launcher_profiles_path,
                               const nl::json& new_launcher_profiles_json, std::error_code* ec);

}  // namespace

struct LauncherProfilesEditor::Data_ {
  fs::path launcher_profiles_path;
  nl::json launcher_profiles_json;
  std::map<std::string, ProfileData> profile_data_map;
};

LauncherProfilesEditor::LauncherProfilesEditor()
    : data_(std::make_unique<LauncherProfilesEditor::Data_>())
{
  // Do nothing
}

LauncherProfilesEditor::Ptr LauncherProfilesEditor::Create(const fs::path& launcher_profiles_path,
                                                           std::error_code* ec)
{
  auto lpe_ptr = Ptr(new LauncherProfilesEditor());
  lpe_ptr->data_->launcher_profiles_path = launcher_profiles_path;
  lpe_ptr->data_->launcher_profiles_json = nl::json::object();
  if (!lpe_ptr->Refresh(ec)) {
    return nullptr;
  }
  return lpe_ptr;
}

bool LauncherProfilesEditor::Refresh(std::error_code* ec)
{
  data_->launcher_profiles_json = nl::json::object();
  data_->profile_data_map.clear();
  if (!fs::exists(data_->launcher_profiles_path)) {
    SetError(ec, Error::LAUNCHER_PROFILES_NONEXISTENT);
    return false;
  }
  std::ifstream launcher_profiles_ifs(data_->launcher_profiles_path);
  nl::json new_launcher_profiles_json = nl::json::parse(launcher_profiles_ifs, nullptr, false);
  launcher_profiles_ifs.close();
  if (new_launcher_profiles_json.is_discarded()) {
    SetError(ec, Error::LAUNCHER_PROFILES_PARSE_FAILED);
    return false;
  }
  data_->launcher_profiles_json = new_launcher_profiles_json;
  const nl::json profiles_json = new_launcher_profiles_json.value("profiles", nl::json::object());
  for (const auto& [profile_id, profile_json] : profiles_json.items()) {
    const nl::json name_json = profile_json.value("name", nl::json(nullptr));
    const nl::json type_json = profile_json.value("type", nl::json(nullptr));
    const nl::json icon_json = profile_json.value("icon", nl::json(nullptr));
    const nl::json version_json = profile_json.value("lastVersionId", nl::json(nullptr));
    const nl::json game_path_json = profile_json.value("gameDir", nl::json(nullptr));
    const nl::json java_path_json = profile_json.value("javaDir", nl::json(nullptr));
    const nl::json created_time_json = profile_json.value("created", nl::json(nullptr));
    const nl::json last_used_time_json = profile_json.value("lastUsed", nl::json(nullptr));
    ProfileData profile_data;
    profile_data.id = profile_id;
    if (name_json.is_string()) {
      profile_data.name_opt = name_json.get<std::string>();
    }
    if (type_json.is_string()) {
      profile_data.type_opt = type_json.get<std::string>();
    }
    if (icon_json.is_string()) {
      profile_data.icon_opt = icon_json.get<std::string>();
    }
    if (version_json.is_string()) {
      profile_data.version_opt = version_json.get<std::string>();
    }
    if (game_path_json.is_string()) {
      profile_data.game_path_opt = game_path_json.get<std::string>();
    }
    if (java_path_json.is_string()) {
      profile_data.java_path_opt = java_path_json.get<std::string>();
    }
    if (created_time_json.is_string()) {
      const std::string created_time_str = created_time_json.get<std::string>();
      profile_data.created_time_opt = TimeFromString(created_time_json);
    }
    if (last_used_time_json.is_string()) {
      const std::string last_used_time_str = last_used_time_json.get<std::string>();
      profile_data.last_used_time_opt = TimeFromString(last_used_time_json);
    }
    data_->profile_data_map.emplace(profile_id, profile_data);
  }
  return true;
}

std::optional<ProfileData> LauncherProfilesEditor::GetProfile(const std::string& id) const
{
  const auto profile_data_iter = data_->profile_data_map.find(id);
  if (profile_data_iter == data_->profile_data_map.end()) {
    return std::nullopt;
  }
  return std::get<1>(*profile_data_iter);
}

std::vector<ProfileData> LauncherProfilesEditor::GetProfiles() const
{
  std::vector<ProfileData> profile_datas;
  for (const auto& [_, profile_data] : data_->profile_data_map) {
    profile_datas.push_back(profile_data);
  }
  // Negate the comparison to reverse the order of the list as we sort
  const auto last_used_comp = [](const ProfileData& aa, const ProfileData& bb) {
    if (aa.last_used_time_opt.has_value() && bb.last_used_time_opt.has_value()) {
      return aa.last_used_time_opt.value() >= bb.last_used_time_opt.value();
    }
    else if (!aa.last_used_time_opt.has_value() && bb.last_used_time_opt.has_value()) {
      return false;
    }
    else if (aa.last_used_time_opt.has_value() && !bb.last_used_time_opt.has_value()) {
      return true;
    }
    return aa.id >= bb.id;
  };
  std::sort(profile_datas.begin(), profile_datas.end(), last_used_comp);
  return profile_datas;
}

bool LauncherProfilesEditor::HasProfileWithId(const std::string& id) const
{
  const auto profile_data_iter = data_->profile_data_map.find(id);
  return profile_data_iter != data_->profile_data_map.end();
}

bool LauncherProfilesEditor::HasProfileWithName(const std::string& name) const
{
  for (const auto& [_, profile_data] : data_->profile_data_map) {
    if (profile_data.name_opt.value_or("") == name) {
      return true;
    }
  }
  return false;
}

std::string LauncherProfilesEditor::GetNewUniqueId() const
{
  // There's an extremely slim chance it's not actually unique
  std::string id = GetRandomId();
  for (std::size_t ii = 0; HasProfileWithId(id) && ii < 1000; ++ii) {
    id = GetRandomId();
  }
  return id;
}

std::string LauncherProfilesEditor::GetNewUniqueName() const
{
  // There's an extremely slim chance it's not actually unique
  std::string name = GetRandomName();
  for (std::size_t ii = 0; HasProfileWithName(name) && ii < 1000; ++ii) {
    name = GetRandomName();
  }
  return name;
}

bool LauncherProfilesEditor::PatchForgeProfile(std::error_code* ec)
{
  if (!Refresh(ec)) {
    return false;
  }
  nl::json forge_profile =
      data_->launcher_profiles_json["profiles"].value("forge", nl::json(nullptr));
  if (!forge_profile.is_object()) {
    SetError(ec, Error::LAUNCHER_PROFILES_NO_FORGE_PROFILE);
    return false;
  }
  // Add a "lastUsed" time because Forge is lazy and doesn't do this!
  forge_profile["lastUsed"] = GetCurrentTimeAsString(std::chrono::seconds(-1));
  nl::json new_launcher_profiles_json = data_->launcher_profiles_json;
  new_launcher_profiles_json["profiles"]["forge"] = forge_profile;
  if (!WriteLauncherProfilesJson(data_->launcher_profiles_path, new_launcher_profiles_json, ec)) {
    return false;
  }
  return true;
}

bool LauncherProfilesEditor::WriteProfile(const ProfileData& profile_data, std::error_code* ec)
{
  if (!Refresh(ec)) {
    return false;
  }
  if (!profile_data.name_opt || !profile_data.name_opt || !profile_data.icon_opt
      || !profile_data.version_opt || !profile_data.game_path_opt) {
    SetError(ec, Error::LAUNCHER_PROFILES_INVALID_PROFILE);
    return false;
  }
  if (HasProfileWithId(profile_data.id)) {
    SetError(ec, Error::LAUNCHER_PROFILES_ID_USED);
    return false;
  }
  if (HasProfileWithName(profile_data.name_opt.value())) {
    SetError(ec, Error::LAUNCHER_PROFILES_NAME_USED);
    return false;
  }
  // Formatting example (as of format 21):
  // "mjrianz5n6o0ntue4gvzfu9zi7i8lg4y": {
  //   "created": "2019-12-12T03:11:18.000Z",
  //   "gameDir" : "/home/tim/.minecraft/trollauncher/Adakite 58",
  //   "icon": "TNT",
  //   "javaDir" : "/usr/lib/jvm/java-8-openjdk-amd64/bin/java",
  //   "lastUsed": "2019-12-12T03:11:18.000Z",
  //   "lastVersionId": "1.14.4-forge-28.1.106",
  //   "name": "Adakite 58",
  //   "type": "custom"
  // },
  const auto now_time = std::chrono::system_clock::now();
  nl::json profile_json = nl::json::object();
  profile_json["name"] = profile_data.name_opt.value();
  profile_json["type"] = profile_data.type_opt.value_or("custom");
  profile_json["icon"] = profile_data.icon_opt.value();
  profile_json["lastVersionId"] = profile_data.version_opt.value();
  profile_json["gameDir"] = profile_data.game_path_opt.value().string();
  profile_json["created"] = StringFromTime(profile_data.created_time_opt.value_or(now_time));
  profile_json["lastUsed"] = StringFromTime(profile_data.last_used_time_opt.value_or(now_time));
  if (profile_data.java_path_opt) {
    profile_json["javaDir"] = profile_data.java_path_opt.value().string();
  }
  nl::json new_launcher_profiles_json = data_->launcher_profiles_json;
  new_launcher_profiles_json["profiles"][profile_data.id] = profile_json;
  if (!WriteLauncherProfilesJson(data_->launcher_profiles_path, new_launcher_profiles_json, ec)) {
    return false;
  }
  return true;
}

bool LauncherProfilesEditor::UpdateProfile(const ProfileData& profile_data, std::error_code* ec)
{
  if (!Refresh(ec)) {
    return false;
  }
  // Use the original JSON here instead of using "GetProfile", so the new JSON
  // better preserves whatever it had going on before the updates
  nl::json profile_json =
      data_->launcher_profiles_json["profiles"].value(profile_data.id, nl::json(nullptr));
  if (!profile_json.is_object()) {
    SetError(ec, Error::LAUNCHER_PROFILES_NO_PROFILE);
    return false;
  }
  const auto now_time = std::chrono::system_clock::now();
  if (profile_data.name_opt) {
    profile_json["name"] = profile_data.name_opt.value();
  }
  if (profile_data.type_opt) {
    profile_json["type"] = profile_data.type_opt.value();
  }
  if (profile_data.icon_opt) {
    profile_json["icon"] = profile_data.icon_opt.value();
  }
  if (profile_data.version_opt) {
    profile_json["lastVersionId"] = profile_data.version_opt.value();
  }
  if (profile_data.game_path_opt) {
    profile_json["gameDir"] = profile_data.game_path_opt.value().string();
  }
  if (profile_data.java_path_opt) {
    profile_json["javaDir"] = profile_data.java_path_opt.value().string();
  }
  if (profile_data.created_time_opt) {
    profile_json["created"] = StringFromTime(profile_data.created_time_opt.value());
  }
  // Always update the last used time, falling back to the current time
  profile_json["lastUsed"] = StringFromTime(profile_data.last_used_time_opt.value_or(now_time));
  nl::json new_launcher_profiles_json = data_->launcher_profiles_json;
  new_launcher_profiles_json["profiles"][profile_data.id] = profile_json;
  if (!WriteLauncherProfilesJson(data_->launcher_profiles_path, new_launcher_profiles_json, ec)) {
    return false;
  }
  return true;
}

namespace {

std::string GetCurrentTimeAsString(std::optional<std::chrono::seconds> time_modifier_opt)
{
  const auto now_time = std::chrono::system_clock::now();
  return StringFromTime(now_time + time_modifier_opt.value_or(std::chrono::seconds(0)));
}

bool IsFileWritable(const fs::path& path)
{
  if (!fs::is_regular_file(path)) {
    return false;
  }
  std::ofstream file(path, std::ios_base::app);
  return file.good();
}

fs::path AddFilenamePrefix(const fs::path& path, const std::string& prefix)
{
  const fs::path new_filename = fs::path(prefix) += path.filename();
  return fs::path(path).replace_filename(new_filename);
}

bool WriteLauncherProfilesJson(const fs::path& launcher_profiles_path,
                               const nl::json& new_launcher_profiles_json, std::error_code* ec)
{
  if (!IsFileWritable(launcher_profiles_path)) {
    SetError(ec, Error::LAUNCHER_PROFILES_NOT_WRITABLE);
    return false;
  }
  const std::string new_launcher_profiles_txt =
      new_launcher_profiles_json.dump(2, ' ', false, nl::json::error_handler_t::replace);
  const fs::path orig_lp_path = launcher_profiles_path;
  const fs::path backup_lp_path = AddFilenamePrefix(orig_lp_path, "backup_");
  const fs::path new_lp_path = AddFilenamePrefix(orig_lp_path, "new_");
  std::error_code fs_ec;
  if (!ITS_A_UNIX_SYSTEM) {
    // Apparently overwrite doesn't work on Windoze!
    fs::remove(backup_lp_path, fs_ec);
  }
  fs::copy_file(orig_lp_path, backup_lp_path, fs::copy_options::overwrite_existing, fs_ec);
  if (fs_ec) {
    SetError(ec, Error::LAUNCHER_PROFILES_BACKUP_FAILED);
    return false;
  }
  std::ofstream new_launcher_profiles_file(new_lp_path);
  if (!new_launcher_profiles_file.good()) {
    SetError(ec, Error::LAUNCHER_PROFILES_WRITE_FAILED);
    return false;
  }
  new_launcher_profiles_file << new_launcher_profiles_txt;
  new_launcher_profiles_file.close();
  if (!ITS_A_UNIX_SYSTEM) {
    // Apparently overwrite doesn't work on Windoze!
    fs::remove(orig_lp_path, fs_ec);
  }
  fs::copy_file(new_lp_path, orig_lp_path, fs::copy_options::overwrite_existing, fs_ec);
  if (fs_ec) {
    SetError(ec, Error::LAUNCHER_PROFILES_WRITE_FAILED);
    return false;
  }
  fs::remove(new_lp_path, fs_ec);
  return true;
}

}  // namespace

}  // namespace tl
