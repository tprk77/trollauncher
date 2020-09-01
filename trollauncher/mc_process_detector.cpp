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

#include "trollauncher/mc_process_detector.hpp"

#include <iostream>
#include <regex>

#ifndef ITS_A_UNIX_SYSTEM
#ifndef _WIN32
#define ITS_A_UNIX_SYSTEM true
#else
#define ITS_A_UNIX_SYSTEM false
#endif
#endif

#if ITS_A_UNIX_SYSTEM
#include <proc/readproc.h>
#else
// TODO
#endif

namespace tl {

namespace {

#if ITS_A_UNIX_SYSTEM
void ForEachProcesses(const std::function<bool(const std::string&)>& func)
{
  PROCTAB* const proctab_ptr = openproc(PROC_FILLCOM);
  if (proctab_ptr == nullptr) {
    // Technically an error, but we will just assume it's not running since
    // detection is not perfect and best effort only
    return;
  }
  proc_t proc_info = {};
  while (readproc(proctab_ptr, &proc_info) != nullptr) {
    if (proc_info.cmdline == nullptr) {
      // Ignore any process without a command line
      continue;
    }
    std::string command_line;
    {
      const char* const* cmdline_ptr = proc_info.cmdline;
      command_line = *cmdline_ptr;
      while (*++cmdline_ptr != nullptr) {
        command_line += " ";
        command_line += *cmdline_ptr;
      }
    }
    if (!func(command_line)) {
      break;
    }
  }
  closeproc(proctab_ptr);
}
#else
void ForEachProcesses(const std::function<bool(const std::string&)>&)
{
  // TODO
}
#endif

}  // namespace

McProcessRunning McProcessDetector::GetRunningMinecraft()
{
  // For the launcher, we attempt to match the absolute path of the program,
  // which *should* be pretty consistent. If the user is doing something
  // slightly wacky like using a relative path, this will fail. This is still
  // kind of the best option because matching just "minecraft-launcher" is a bit
  // generic and might give false positives.
  const std::regex launcher_regex("^\\/opt\\/minecraft-launcher\\/minecraft-launcher");
  // For the game, we want to specifically match instances of Minecraft launched
  // by the launcher. Luckily the launcher always adds an argument to the
  // command, so we can use that. To cut down on false positives, we also match
  // the main class, which should always be one of the three listed below.
  const std::regex game_launch_regex("-Dminecraft\\.launcher\\.brand=minecraft-launcher");
  const std::regex game_class_regex(
      "net\\.minecraft\\.client\\.main\\.Main"
      "|cpw\\.mods\\.modlauncher\\.Launcher"
      "|net\\.minecraft\\.launchwrapper\\.Launch");
  bool found_launcher = false;
  bool found_game = false;
  ForEachProcesses([&](const std::string& command_line) {
    if (found_launcher && found_game) {
      // Stop searching early if we already found both
      return false;
    }
    if (!found_launcher) {
      if (std::regex_search(command_line, launcher_regex)) {
        found_launcher = true;
        return true;
      }
    }
    if (!found_game) {
      if (std::regex_search(command_line, game_launch_regex)
          && std::regex_search(command_line, game_class_regex)) {
        found_game = true;
        return true;
      }
    }
    return true;
  });
  return (found_launcher && found_game
              ? McProcessRunning::LAUNCHER_AND_GAME
              : (found_launcher ? McProcessRunning::LAUNCHER
                                : (found_game ? McProcessRunning::GAME : McProcessRunning::NONE)));
}

}  // namespace tl
