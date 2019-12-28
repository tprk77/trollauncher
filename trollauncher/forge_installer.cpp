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

#include "trollauncher/forge_installer.hpp"

#include <fstream>
#include <sstream>

#include <libzippp.h>
#include <nlohmann/json.hpp>

// Work around for Boost on MSYS2
#ifdef _WIN32
#ifndef __kernel_entry
#define __kernel_entry
#endif
#endif

#include <boost/process.hpp>

#include "trollauncher/error_codes.hpp"
#include "trollauncher/java_detector.hpp"

namespace tl {

namespace {

namespace bp = boost::process;
namespace fs = std::filesystem;
namespace nl = nlohmann;
namespace zpp = libzippp;

}  // namespace

struct ForgeInstaller::Data_ {
  fs::path installer_path;
  fs::path dot_minecraft_path;
  std::string forge_version;
  std::string minecraft_version;
  std::unique_ptr<zpp::ZipArchive> zip_ptr;
};

ForgeInstaller::ForgeInstaller() : data_(std::make_unique<ForgeInstaller::Data_>())
{
  // Do nothing
}

ForgeInstaller::Ptr ForgeInstaller::Create(const fs::path& installer_path,
                                           const std::filesystem::path& dot_minecraft_path,
                                           std::error_code* ec)
{
  if (!fs::exists(installer_path)) {
    SetError(ec, Error::FORGE_INSTALLER_NONEXISTENT);
    return nullptr;
  }
  // TODO Test if this is a jar file
  if (!fs::is_regular_file(installer_path)) {
    SetError(ec, Error::FORGE_INSTALLER_NOT_REGULAR_FILE);
    return nullptr;
  }
  auto zip_ptr = std::make_unique<zpp::ZipArchive>(installer_path.string());
  if (!zip_ptr->open(zpp::ZipArchive::READ_ONLY)) {
    SetError(ec, Error::FORGE_INSTALLER_JAR_OPEN_FAILED);
    return nullptr;
  }
  zpp::ZipEntry version_entry = zip_ptr->getEntry("version.json", true);
  if (version_entry.isNull()) {
    SetError(ec, Error::FORGE_INSTALLER_NO_VERSION_JSON);
    return nullptr;
  }
  std::stringstream version_ss;
  const int zip_error_code = zip_ptr->readEntry(version_entry, version_ss);
  if (zip_error_code != LIBZIPPP_OK) {
    SetError(ec, Error::FORGE_INSTALLER_VERSION_JSON_READ_FAILED);
    return nullptr;
  }
  nl::json version_json = nl::json::parse(version_ss.str(), nullptr, false);
  if (version_json.is_discarded()) {
    SetError(ec, Error::FORGE_INSTALLER_VERSION_JSON_PARSE_FAILED);
    return nullptr;
  }
  const std::string forge_version = version_json.value("id", "");
  const std::string minecraft_version = version_json.value("inheritsFrom", "");
  if (forge_version.empty() || minecraft_version.empty()) {
    SetError(ec, Error::FORGE_INSTALLER_BAD_VERSION_JSON);
    return nullptr;
  }
  auto fi_ptr = Ptr(new ForgeInstaller());
  fi_ptr->data_->installer_path = installer_path;
  fi_ptr->data_->dot_minecraft_path = dot_minecraft_path;
  fi_ptr->data_->forge_version = forge_version;
  fi_ptr->data_->minecraft_version = minecraft_version;
  fi_ptr->data_->zip_ptr = std::move(zip_ptr);
  return fi_ptr;
}

std::string ForgeInstaller::GetForgeVersion() const
{
  // E.g., "1.14.4-forge-28.1.109"
  return data_->forge_version;
}

std::string ForgeInstaller::GetMinecraftVersion() const
{
  // E.g., "1.14.4"
  return data_->minecraft_version;
}

bool ForgeInstaller::IsInstalled() const
{
  // Look for ".minecraft/versions/${forge_version}/${forge_version}.json
  const fs::path installed_version_path = data_->dot_minecraft_path / "versions"
                                          / data_->forge_version / (data_->forge_version + ".json");
  return fs::exists(installed_version_path);
}

bool ForgeInstaller::Install(std::error_code* ec)
{
  const std::optional<fs::path> java_path_opt = JavaDetector::GetAnyJava();
  if (!java_path_opt) {
    SetError(ec, Error::FORGE_INSTALLER_NO_JAVA);
    return false;
  }
  std::error_code java_ec;
  const int return_code = bp::system(                       //
      bp::exe = java_path_opt.value().string(),             //
      bp::args = {"-jar", data_->installer_path.string()},  //
      (bp::std_out & bp::std_err) > bp::null,               //
      bp::error = java_ec                                   //
  );
  if (java_ec) {
    SetError(ec, Error::FORGE_INSTALLER_EXECUTE_FAILED);
    return false;
  }
  if (return_code != 0) {
    SetError(ec, Error::FORGE_INSTALLER_INSTALL_FAILED);
    return false;
  }
  // Sanity check that the installer actually worked
  if (!IsInstalled()) {
    SetError(ec, Error::FORGE_INSTALLER_BAD_INSTALL);
    return false;
  }
  return true;
}

}  // namespace tl
