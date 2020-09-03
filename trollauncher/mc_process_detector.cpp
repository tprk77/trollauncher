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
#define _WIN32_DCOM
#include <comutil.h>
#include <wbemidl.h>
#include <windows.h>
#include "TlHelp32.h"
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
std::optional<std::string> StringFromBSTR(const _bstr_t& bstr)
{
  if (bstr.length() == 0) {
    return "";
  }
  const int wide_len = SysStringLen(bstr);
  const int req_len = WideCharToMultiByte(CP_UTF8, 0, bstr, wide_len, nullptr, 0, nullptr, nullptr);
  if (req_len == 0) {
    return std::nullopt;
  }
  std::string str(req_len, '\0');
  const int str_len =
      WideCharToMultiByte(CP_UTF8, 0, bstr, wide_len, str.data(), req_len, nullptr, nullptr);
  if (str_len == 0) {
    return std::nullopt;
  }
  return str;
}

class ScopedExiter {
 public:
  ScopedExiter(const std::function<void()>& func) : func_(func) {}
  ~ScopedExiter()
  {
    func_();
  }

 private:
  std::function<void()> func_;
};

void ForEachProcesses(const std::function<bool(const std::string&)>& func)
{
  // This only needs to be done once per thread, but it's also ok to do it
  // multiple times, which is what we do here. The problem is when it's called
  // with a different concurrency model, which seems to happen with wxWidgets.
  // In that case, we don't care if it fails.
  const HRESULT hr_init = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hr_init) && hr_init != RPC_E_CHANGED_MODE) {
    return;
  }
  const ScopedExiter uninitializer([]() { CoUninitialize(); });
  // This is supposed to be called exactly once. So if it's already been called,
  // detect that and just ignore the error.
  const HRESULT hr_init_sec =
      CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
                           RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
  if (FAILED(hr_init_sec) && hr_init_sec != RPC_E_TOO_LATE) {
    return;
  }
  IWbemLocator* wbem_locator = nullptr;
  if (FAILED(CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                              reinterpret_cast<LPVOID*>(&wbem_locator)))) {
    return;
  }
  const ScopedExiter wbem_locator_releaser([&]() { wbem_locator->Release(); });
  IWbemServices* wbem_services = nullptr;
  if (FAILED(wbem_locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0,
                                         nullptr, nullptr, &wbem_services))) {
    return;
  }
  const ScopedExiter wbem_services_releaser([&]() { wbem_services->Release(); });
  IEnumWbemClassObject* enum_wbem = nullptr;
  if (FAILED(wbem_services->ExecQuery(_bstr_t(L"WQL"),
                                      _bstr_t(L"SELECT CommandLine FROM Win32_Process"),
                                      WBEM_FLAG_FORWARD_ONLY, nullptr, &enum_wbem))) {
    return;
  }
  const ScopedExiter enum_wbem_releaser([&]() { enum_wbem->Release(); });
  IWbemClassObject* wbem_object = nullptr;
  ULONG num_objects = 0;
  // Don't use SUCCEEDED here, but specifically check for S_OK
  while (enum_wbem->Next(static_cast<long>(WBEM_INFINITE), 1, &wbem_object, &num_objects) == S_OK) {
    if (num_objects == 0) {
      continue;
    }
    const ScopedExiter wbem_object_releaser([&]() { wbem_object->Release(); });
    _variant_t command_line_var;
    if (FAILED(wbem_object->Get(L"CommandLine", 0, &command_line_var, nullptr, nullptr))
        || command_line_var.vt != VT_BSTR) {
      continue;
    }
    const std::string command_line = *StringFromBSTR(static_cast<_bstr_t>(command_line_var));
    if (!func(command_line)) {
      break;
    }
  }
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
  const std::regex launcher_regex(ITS_A_UNIX_SYSTEM
                                      ? "^\\/opt\\/minecraft-launcher\\/minecraft-launcher"
                                      : "\\\\Minecraft Launcher\\\\MinecraftLauncher\\.exe");
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
