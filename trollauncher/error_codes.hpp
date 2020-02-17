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

#ifndef TROLLAUNCHER_ERROR_CODES_HPP_
#define TROLLAUNCHER_ERROR_CODES_HPP_

#include <string>
#include <system_error>

namespace tl {

enum class Error {
  OK = 0,
  DOT_MINECRAFT_NO_DEFAULT,
  DOT_MINECRAFT_NONEXISTENT,
  LAUNCHER_PROFILES_NONEXISTENT,
  LAUNCHER_PROFILES_PARSE_FAILED,
  LAUNCHER_PROFILES_NO_FORGE_PROFILE,
  LAUNCHER_PROFILES_ID_USED,
  LAUNCHER_PROFILES_NAME_USED,
  LAUNCHER_PROFILES_NOT_WRITABLE,
  LAUNCHER_PROFILES_BACKUP_FAILED,
  LAUNCHER_PROFILES_WRITE_FAILED,
  MODPACK_NONEXISTENT,
  MODPACK_NOT_REGULAR_FILE,
  MODPACK_ZIP_OPEN_FAILED,
  MODPACK_PREP_INSTALL_TEMPDIR_FAILED,
  MODPACK_PREP_INSTALL_UNZIP_FAILED,
  MODPACK_DESTINATION_CREATION_FAILED,
  MODPACK_DESTINATION_NOT_DIRECTORY,
  MODPACK_DESTINATION_NOT_EMPTY,
  MODPACK_KEEPLIST_FAILED,
  MODPACK_UNZIP_FAILED,
  FORGE_INSTALLER_NONEXISTENT,
  FORGE_INSTALLER_NOT_REGULAR_FILE,
  FORGE_INSTALLER_JAR_OPEN_FAILED,
  FORGE_INSTALLER_NO_INSTALL_PROFILE_JSON,
  FORGE_INSTALLER_INSTALL_PROFILE_JSON_READ_FAILED,
  FORGE_INSTALLER_INSTALL_PROFILE_JSON_PARSE_FAILED,
  FORGE_INSTALLER_BAD_INSTALL_PROFILE_JSON,
  FORGE_INSTALLER_NO_JAVA,
  FORGE_INSTALLER_EXECUTE_FAILED,
  FORGE_INSTALLER_INSTALL_FAILED,
  FORGE_INSTALLER_BAD_INSTALL,
  PROFILE_NONEXISTENT,
  PROFILE_NOT_AN_INSTALL,
  PROFILE_GET_FILES_FAILED,
  PROFILE_BACKUP_FAILED,
};

std::error_code MakeErrorCode(Error error);
void SetError(std::error_code* ec, Error error);

class TrollauncherCategory final : public std::error_category {
 public:
  TrollauncherCategory(const TrollauncherCategory&) = delete;
  ~TrollauncherCategory() override = default;

  static const TrollauncherCategory& GetInstance();

  TrollauncherCategory& operator=(const TrollauncherCategory&) = delete;

  const char* name() const noexcept override;
  std::string message(int condition) const override;

 private:
  constexpr TrollauncherCategory() = default;
};

}  // namespace tl

#endif  // TROLLAUNCHER_ERROR_CODES_HPP_
