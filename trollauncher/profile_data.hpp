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

#ifndef TROLLAUNCHER_PROFILE_DATA_HPP_
#define TROLLAUNCHER_PROFILE_DATA_HPP_

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

namespace tl {

// The only truely required fields are apparently ID, name, and type. (ID is used as the key to the
// object, and the Minecraft Launcher will automatically add an empty name and type.)

struct ProfileData {
  std::string id;
  std::optional<std::string> name_opt;
  std::optional<std::string> type_opt;
  std::optional<std::string> icon_opt;
  std::optional<std::string> version_opt;
  std::optional<std::filesystem::path> game_path_opt;
  std::optional<std::filesystem::path> java_path_opt;
  std::optional<std::chrono::system_clock::time_point> created_time_opt;
  std::optional<std::chrono::system_clock::time_point> last_used_time_opt;
};

}  // namespace tl

#endif  // TROLLAUNCHER_PROFILE_DATA_HPP_
