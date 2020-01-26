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

#include "trollauncher/error_codes.hpp"

namespace tl {

std::error_code MakeErrorCode(Error error)
{
  return std::error_code(static_cast<int>(error), TrollauncherCategory::GetInstance());
}

void SetError(std::error_code* ec, Error error)
{
  if (ec != nullptr) {
    *ec = MakeErrorCode(error);
  }
}

const TrollauncherCategory& TrollauncherCategory::GetInstance()
{
  static const class TrollauncherCategory trollauncher_category;
  return trollauncher_category;
}

const char* TrollauncherCategory::name() const noexcept
{
  return "trollauncher";
}

std::string TrollauncherCategory::message(int error) const
{
  if (error == static_cast<int>(Error::OK)) {
    return "Actually everything is fine, lol";
  }
  if (error == static_cast<int>(Error::DOT_MINECRAFT_NO_DEFAULT)) {
    return "Failed to detect default '.minecraft' directory";
  }
  else if (error == static_cast<int>(Error::DOT_MINECRAFT_NONEXISTENT)) {
    return "The '.minecraft' directory does not exist";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_NONEXISTENT)) {
    return "Launcher profiles file does not exist";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_PARSE_FAILED)) {
    return "Failed to parse JSON of launcher profiles file";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_NO_FORGE_PROFILE)) {
    return "Launcher profiles file is missing the Forge profile";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_ID_USED)) {
    return "Profile ID is not unique";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_NAME_USED)) {
    return "Profile name is not unique";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_NOT_WRITABLE)) {
    return "Launcher profiles file is not writable";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_BACKUP_FAILED)) {
    return "Failed to backup launcher profiles file";
  }
  else if (error == static_cast<int>(Error::LAUNCHER_PROFILES_WRITE_FAILED)) {
    return "Failed to write to launcher profiles file";
  }
  else if (error == static_cast<int>(Error::MODPACK_NONEXISTENT)) {
    return "Modpack zip file does not exist";
  }
  else if (error == static_cast<int>(Error::MODPACK_NOT_REGULAR_FILE)) {
    return "Modpack zip file is not a regular file";
  }
  else if (error == static_cast<int>(Error::MODPACK_ZIP_OPEN_FAILED)) {
    return "Failed to open modpack zip file";
  }
  else if (error == static_cast<int>(Error::MODPACK_PREP_INSTALL_TEMPDIR_FAILED)) {
    return "Failed to create temporary directory while preparing for modpack install";
  }
  else if (error == static_cast<int>(Error::MODPACK_PREP_INSTALL_UNZIP_FAILED)) {
    return "Failed to unzip while preparing for modpack install";
  }
  else if (error == static_cast<int>(Error::MODPACK_DESTINATION_CREATION_FAILED)) {
    return "Failed to create directory for modpack install destination";
  }
  else if (error == static_cast<int>(Error::MODPACK_DESTINATION_NOT_DIRECTORY)) {
    return "The modpack install destination is not a directory";
  }
  else if (error == static_cast<int>(Error::MODPACK_DESTINATION_NOT_EMPTY)) {
    return "The modpack install destination is not an empty directory";
  }
  else if (error == static_cast<int>(Error::MODPACK_UNZIP_FAILED)) {
    return "Failed to unzip the modpack zip file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_NONEXISTENT)) {
    return "The Forge installer does not exist";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_NOT_REGULAR_FILE)) {
    return "The Forge installer is not a regular file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_JAR_OPEN_FAILED)) {
    return "Failed to open Forge installer jar file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_NO_INSTALL_PROFILE_JSON)) {
    return "Forge installer jar file does not contain install_profile.json";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_INSTALL_PROFILE_JSON_READ_FAILED)) {
    return "Failed to read install_profile.json from the Forge installer jar file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_INSTALL_PROFILE_JSON_PARSE_FAILED)) {
    return "Failed to parse install_profile.json from the Forge installer jar file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_BAD_INSTALL_PROFILE_JSON)) {
    return "Bad contents in install_profile.json from the Forge installer jar file";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_NO_JAVA)) {
    return "No Java to run the Forge installer";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_EXECUTE_FAILED)) {
    return "Failed to execute the Forge installer";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_INSTALL_FAILED)) {
    return "Forge installer failed to install";
  }
  else if (error == static_cast<int>(Error::FORGE_INSTALLER_BAD_INSTALL)) {
    return "Forge installer ran, but didn't install correctly";
  }
  else {
    return "Unknown Trollauncher error";
  }
}

}  // namespace tl
