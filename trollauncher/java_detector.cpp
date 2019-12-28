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

#include "trollauncher/java_detector.hpp"

#include <optional>
#include <regex>
#include <vector>

// Work around for Boost on MSYS2
#ifdef _WIN32
#ifndef __kernel_entry
#define __kernel_entry
#endif
#endif

#include <boost/process.hpp>

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

namespace bfs = boost::filesystem;
namespace bp = boost::process;
namespace fs = std::filesystem;

std::vector<fs::path> GetProgramFilesPaths();
std::vector<fs::path> GetPrefixedPaths(const std::vector<fs::path>& prefix_paths,
                                       const fs::path& relative_path);
std::optional<std::string> GetJavaVersion(const fs::path& java_path);
bool CheckJavaVersion(const fs::path& java_path,
                      const std::optional<std::regex>& version_regex_opt);
std::optional<fs::path> FindLinuxJava(const std::optional<std::regex>& version_regex_opt);
std::optional<fs::path> FindWindowsJava(const std::vector<fs::path>& program_files_paths,
                                        const std::optional<std::regex>& version_regex_opt);
std::optional<fs::path> FindBundledJava(const std::vector<fs::path>& program_files_paths,
                                        const std::optional<std::regex>& version_regex_opt);
std::optional<fs::path> FindJava(const std::optional<std::regex>& version_regex_opt);

}  // namespace

std::optional<fs::path> JavaDetector::GetAnyJava()
{
  return FindJava(std::nullopt);
}

std::optional<fs::path> JavaDetector::GetJavaVersion8()
{
  const std::regex version_8_regex("^1\\.8\\.[0-9]+");
  return FindJava(version_8_regex);
}

namespace {

std::vector<fs::path> GetProgramFilesPaths()
{
  if (ITS_A_UNIX_SYSTEM) {
    return {};
  }
  const std::vector<std::optional<std::string>> program_files_path_opts = {
      GetEnvironmentVar("PROGRAMFILES"), GetEnvironmentVar("PROGRAMFILES(X86)")};
  std::vector<fs::path> program_files_paths;
  for (const auto& program_files_path_opt : program_files_path_opts) {
    if (program_files_path_opt) {
      program_files_paths.push_back(program_files_path_opt.value());
    }
  }
  // Remove duplicate entries
  if (program_files_paths.size() == 2 && program_files_paths.at(0) == program_files_paths.at(1)) {
    program_files_paths.pop_back();
  }
  return program_files_paths;
}

std::vector<fs::path> GetPrefixedPaths(const std::vector<fs::path>& prefix_paths,
                                       const fs::path& relative_path)
{
  std::vector<fs::path> full_paths;
  for (const auto& prefix_path : prefix_paths) {
    full_paths.push_back(prefix_path / relative_path);
  }
  return full_paths;
}

std::optional<std::string> GetJavaVersion(const fs::path& java_path)
{
  if (!fs::is_regular_file(java_path)) {
    return std::nullopt;
  }
  bp::ipstream java_ips;
  std::error_code java_ec;
  const int return_code = bp::system(          //
      bp::exe = java_path.string(),            //
      bp::args = {"-version"},                 //
      (bp::std_out & bp::std_err) > java_ips,  //
      bp::error = java_ec                      //
  );
  if (java_ec || return_code != 0) {
    return std::nullopt;
  }
  std::string java_output;
  std::getline(java_ips, java_output);
  // Example version strings:
  // java version "1.8.0_51"
  // openjdk version "11.0.5" 2019-10-15
  std::smatch version_match;
  static const std::regex version_regex("^[^ ]+ version \"([^\"]+)\"");
  if (!std::regex_search(java_output, version_match, version_regex)) {
    return std::nullopt;
  }
  return version_match[1];
}

bool CheckJavaVersion(const fs::path& java_path, const std::optional<std::regex>& version_regex_opt)
{
  const std::optional<std::string> java_version_opt = GetJavaVersion(java_path);
  if (!java_version_opt) {
    return false;
  }
  const std::string& java_version = java_version_opt.value();
  if (version_regex_opt && !std::regex_search(java_version, version_regex_opt.value())) {
    return false;
  }
  return true;
}

std::optional<fs::path> FindLinuxJava(const std::optional<std::regex>& version_regex_opt)
{
  if (!ITS_A_UNIX_SYSTEM) {
    return std::nullopt;
  }
  // Example Java paths:
  // /usr/lib/jvm/java-8-openjdk-amd64/bin/java
  // /usr/lib/jvm/java-11-openjdk-amd64/bin/java
  const fs::path java_root_path = "/usr/lib/jvm/";
  std::error_code fs_ec;
  for (const fs::path& java_dir_path : fs::directory_iterator(java_root_path, fs_ec)) {
    if (!fs::is_directory(java_dir_path)) {
      continue;
    }
    const fs::path java_path = java_dir_path / "bin/java";
    if (CheckJavaVersion(java_path, version_regex_opt)) {
      return java_path;
    }
  }
  return std::nullopt;
}

std::optional<fs::path> FindWindowsJava(const std::vector<fs::path>& program_files_paths,
                                        const std::optional<std::regex>& version_regex_opt)
{
  if (ITS_A_UNIX_SYSTEM) {
    return std::nullopt;
  }
  // Example Java path:
  // C:\Program Files\Java\jre1.8.0_231\bin\javaw.exe
  const std::vector<fs::path> java_root_paths = GetPrefixedPaths(program_files_paths, "Java");
  for (const auto& java_root_path : java_root_paths) {
    std::error_code fs_ec;
    for (const fs::path& java_dir_path : fs::directory_iterator(java_root_path, fs_ec)) {
      if (!fs::is_directory(java_dir_path)) {
        continue;
      }
      const fs::path java_path = java_dir_path / "bin\\javaw.exe";
      if (CheckJavaVersion(java_path, version_regex_opt)) {
        return java_path;
      }
    }
  }
  return std::nullopt;
}

std::optional<fs::path> FindBundledJava(const std::vector<fs::path>& program_files_paths,
                                        const std::optional<std::regex>& version_regex_opt)
{
  if (ITS_A_UNIX_SYSTEM) {
    return std::nullopt;
  }
  // Example Java path:
  // C:\Program Files (x86)\Minecraft Launcher\runtime\jre-x64\bin\javaw.exe
  const std::vector<fs::path> bundled_java_paths =
      GetPrefixedPaths(program_files_paths, "Minecraft Launcher\\runtime\\jre-x64\\bin\\javaw.exe");
  for (const fs::path& java_path : bundled_java_paths) {
    if (CheckJavaVersion(java_path, version_regex_opt)) {
      return java_path;
    }
  }
  return std::nullopt;
}

std::optional<fs::path> FindJava(const std::optional<std::regex>& version_regex_opt)
{
  const std::vector<fs::path> program_files_paths = GetProgramFilesPaths();
  const std::optional<fs::path> launcher_java_path_opt =
      FindBundledJava(program_files_paths, version_regex_opt);
  if (launcher_java_path_opt) {
    return launcher_java_path_opt;
  }
  const fs::path path_java_path = bp::search_path("java").string();
  if (CheckJavaVersion(path_java_path, version_regex_opt)) {
    return path_java_path;
  }
  if (ITS_A_UNIX_SYSTEM) {
    return FindLinuxJava(version_regex_opt);
  }
  else {
    return FindWindowsJava(program_files_paths, version_regex_opt);
  }
  return std::nullopt;
}

}  // namespace

}  // namespace tl
