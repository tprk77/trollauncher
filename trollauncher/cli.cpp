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

#include "trollauncher/cli.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>

#include "trollauncher/modpack_installer.hpp"

namespace tl {

namespace {

namespace fs = std::filesystem;
namespace bpo = boost::program_options;

struct InstallArgs {
  std::string modpack_path;
  std::string profile_name;
  std::string profile_icon;
};

struct ListArgs {
  enum class Format { YAML, CSV };
  Format format;
  std::string csv_delim;
};

std::optional<std::string> GetCommand(const int argc, const char* const argv[]);
std::vector<std::string> GetArgs(const int argc, const char* const argv[]);
std::optional<InstallArgs> ParseInstallArgs(const std::vector<std::string>& args,
                                            bool* show_usage_ptr, std::string* error_string_ptr);
std::optional<ListArgs> ParseListArgs(const std::vector<std::string>& args, bool* show_usage_ptr,
                                      std::string* error_string_ptr);
int InstallCli(const InstallArgs& install_args);
int ListCli(const ListArgs& list_args);
std::string QuotedStringOrNull(const std::optional<std::string>& str_opt);
std::string QuotedStringOrNull(const std::optional<fs::path>& path_opt);
void OutputYaml(const std::vector<ProfileData>& profile_datas);
void OutputCsv(const std::vector<ProfileData>& profile_datas, const std::string& delim);

static const std::string overall_help_text =
    ("Usage: trollauncher {install | list | --help} ...\n"
     "\n"
     "Trollauncher is a modpack installer for the \"Vanilla\" Minecraft Launcher.\n"
     "\n"
     "Available subcommands:\n"
     "\n"
     "    install [--help] [--name NAME] [--icon ICON-ID] MODPACK-PATH\n"
     "\n"
     "        Create a new launcher profile from a modpack.\n"
     "\n"
     "    list [--help] [--yaml] [--csv=[DELIM]]\n"
     "\n"
     "        List previously installed launcher profiles.\n"
     "\n"
     "\n"
     "Trollolololololololololo!\n");

static const std::string install_help_text =
    ("Usage: trollauncher install [--help] [--name NAME] [--icon ICON-ID] MODPACK-PATH\n"
     "\n"
     "Create a new profile from a modpack.\n"
     "\n"
     "    --help (-h)             Show install help\n"
     "    --name (-n) NAME        Name of the new profile\n"
     "    --icon (-i) ICON-ID     Icon ID of the new profile\n"
     "    MODPACK-PATH            Path to the modpack zip file\n"
     "\n"
     "\n"
     "Trollolololololololololo!\n");

static const std::string list_help_text =
    ("Usage: trollauncher list [--help] [--yaml] [--csv=[DELIM]]\n"
     "\n"
     "List previously installed launcher profiles.\n"
     "\n"
     "    --help (-h)             Show install help\n"
     "    --yaml (-y)             Output as YAML\n"
     "    --csv=[DELIM] (-c)      Output as CSV (DELIM=',')\n"
     "\n"
     "\n"
     "Trollolololololololololo!\n");

}  // namespace

int CliMain(const int argc, const char* const argv[])
{
  const std::optional<std::string> command_opt = GetCommand(argc, argv);
  const std::vector<std::string> args = GetArgs(argc, argv);
  if (!command_opt) {
    // This shouldn't be possible, because we run the GUI with zero args
    std::cerr << "Durp! Durp! Durp!\n";
    return 1;
  }
  const std::string& command = command_opt.value();
  if (command == "install") {
    bool show_usage = false;
    std::string error_string;
    const std::optional<InstallArgs> install_args_opt =
        ParseInstallArgs(args, &show_usage, &error_string);
    if (!install_args_opt) {
      if (show_usage) {
        std::cerr << install_help_text << "\n";
      }
      else if (!error_string.empty()) {
        std::cerr << "Error: " << error_string << "\n";
      }
      return 1;
    }
    return InstallCli(install_args_opt.value());
  }
  else if (command == "list") {
    bool show_usage = false;
    std::string error_string;
    const std::optional<ListArgs> list_args_opt = ParseListArgs(args, &show_usage, &error_string);
    if (!list_args_opt) {
      if (show_usage) {
        std::cerr << list_help_text << "\n";
      }
      else if (!error_string.empty()) {
        std::cerr << "Error: " << error_string << "\n";
      }
      return 1;
    }
    return ListCli(list_args_opt.value());
  }
  else {
    if (command != "--help" && command != "-h") {
      std::cerr << "Error: Unrecognized command '" << command << "'\n";
    }
    std::cerr << overall_help_text << "\n";
    return 1;
  }
}

namespace {

std::optional<std::string> GetCommand(const int argc, const char* const argv[])
{
  if (argc < 2) {
    return std::nullopt;
  }
  return argv[1];
}

std::vector<std::string> GetArgs(const int argc, const char* const argv[])
{
  std::vector<std::string> args;
  for (int ii = 2; ii < argc; ++ii) {
    args.push_back(argv[ii]);
  }
  return args;
}

std::optional<InstallArgs> ParseInstallArgs(const std::vector<std::string>& args,
                                            bool* show_usage_ptr, std::string* error_string_ptr)
{
  if (show_usage_ptr != nullptr) {
    *show_usage_ptr = false;
  }
  if (error_string_ptr != nullptr) {
    *error_string_ptr = "";
  }
  bpo::options_description options;
  auto ez_adder = options.add_options();
  ez_adder("help,h", new bpo::untyped_value(true));
  ez_adder("name,n", bpo::value<std::string>());
  ez_adder("icon,i", bpo::value<std::string>());
  // Don't make this "required", but check the count later
  ez_adder("path", bpo::value<std::string>());
  bpo::positional_options_description positional;
  positional.add("path", 1);
  bpo::command_line_parser parser(args);
  parser.options(options);
  parser.positional(positional);
  bpo::variables_map vm;
  try {
    bpo::store(parser.run(), vm);
    bpo::notify(vm);
  }
  catch (const bpo::error& ex) {
    if (error_string_ptr != nullptr) {
      *error_string_ptr = ex.what();
    }
    return std::nullopt;
  }
  if (vm.count("help") != 0) {
    if (show_usage_ptr != nullptr) {
      *show_usage_ptr = true;
    }
    if (error_string_ptr != nullptr) {
      *error_string_ptr = install_help_text;
    }
    return std::nullopt;
  }
  if (vm.count("path") == 0) {
    if (error_string_ptr != nullptr) {
      *error_string_ptr = "Missing path to modpack zip file";
    }
    return std::nullopt;
  }
  InstallArgs install_args;
  install_args.modpack_path = vm.at("path").as<std::string>();
  install_args.profile_name = (vm.count("name") != 0 ? vm.at("name").as<std::string>() : "");
  install_args.profile_icon = (vm.count("icon") != 0 ? vm.at("icon").as<std::string>() : "");
  return install_args;
}

std::optional<ListArgs> ParseListArgs(const std::vector<std::string>& args, bool* show_usage_ptr,
                                      std::string* error_string_ptr)
{
  if (show_usage_ptr != nullptr) {
    *show_usage_ptr = false;
  }
  if (error_string_ptr != nullptr) {
    *error_string_ptr = "";
  }
  bpo::options_description options;
  auto ez_adder = options.add_options();
  ez_adder("help,h", new bpo::untyped_value(true));
  ez_adder("yaml,y", new bpo::untyped_value(true));
  ez_adder("csv,c", bpo::value<std::string>()->implicit_value(","));
  bpo::command_line_parser parser(args);
  parser.options(options);
  bpo::variables_map vm;
  try {
    bpo::store(parser.run(), vm);
    bpo::notify(vm);
  }
  catch (const bpo::error& ex) {
    if (error_string_ptr != nullptr) {
      *error_string_ptr = ex.what();
    }
    return std::nullopt;
  }
  if (vm.count("help") != 0) {
    if (show_usage_ptr != nullptr) {
      *show_usage_ptr = true;
    }
    if (error_string_ptr != nullptr) {
      *error_string_ptr = install_help_text;
    }
    return std::nullopt;
  }
  if (vm.count("yaml") != 0 && vm.count("csv") != 0) {
    if (error_string_ptr != nullptr) {
      *error_string_ptr = "You must specify exactly one output format";
    }
    return std::nullopt;
  }
  ListArgs list_args;
  list_args.format = (vm.count("csv") != 0 ? ListArgs::Format::CSV : ListArgs::Format::YAML);
  list_args.csv_delim = (vm.count("csv") != 0 ? vm.at("csv").as<std::string>() : "");
  return list_args;
}

int InstallCli(const InstallArgs& install_args)
{
  std::error_code ec;
  auto mi_ptr = ModpackInstaller::Create(install_args.modpack_path, &ec);
  if (mi_ptr == nullptr) {
    std::cerr << "Error: " << ec.message() << "\n";
    return 1;
  }
  if (!install_args.profile_name.empty()) {
    mi_ptr->SetName(install_args.profile_name);
  }
  if (!install_args.profile_icon.empty()) {
    mi_ptr->SetIcon(install_args.profile_icon);
  }
  if (!mi_ptr->Install(&ec)) {
    std::cerr << "Error: " << ec.message() << "\n";
    return 1;
  }
  std::cerr << "Created profile '" << mi_ptr->GetName()  //
            << "' with icon '" << mi_ptr->GetIcon() << "'\n";
  std::cerr << "Modpack installed successfully!\n";
  return 0;
}

int ListCli(const ListArgs& list_args)
{
  std::error_code ec;
  const std::vector<ProfileData> profile_datas = GetInstalledProfiles(&ec);
  if (ec) {
    std::cerr << "Error: " << ec.message() << "\n";
    return 1;
  }
  switch (list_args.format) {
  case ListArgs::Format::YAML:
    OutputYaml(profile_datas);
    break;
  case ListArgs::Format::CSV:
    OutputCsv(profile_datas, list_args.csv_delim);
    break;
  }
  return 0;
}

std::string QuotedStringOrNull(const std::optional<std::string>& str_opt)
{
  if (!str_opt) {
    return "null";
  }
  std::stringstream ss;
  ss << std::quoted(str_opt.value());
  return ss.str();
}

std::string QuotedStringOrNull(const std::optional<fs::path>& path_opt)
{
  if (!path_opt) {
    return "null";
  }
  std::stringstream ss;
  ss << std::quoted(path_opt.value().string());
  return ss.str();
}

void OutputYaml(const std::vector<ProfileData>& profile_datas)
{
  for (const ProfileData& profile_data : profile_datas) {
    std::cout << profile_data.id << ":\n";
    std::cout << "  name: " << QuotedStringOrNull(profile_data.name_opt) << "\n";
    std::cout << "  type: " << QuotedStringOrNull(profile_data.type_opt) << "\n";
    std::cout << "  icon: " << QuotedStringOrNull(profile_data.icon_opt) << "\n";
    std::cout << "  version: " << QuotedStringOrNull(profile_data.version_opt) << "\n";
    std::cout << "  game_path: " << QuotedStringOrNull(profile_data.game_path_opt) << "\n";
    std::cout << "  java_path: " << QuotedStringOrNull(profile_data.java_path_opt) << "\n";
  }
}

void OutputCsv(const std::vector<ProfileData>& profile_datas, const std::string& delim)
{
  std::cout << "ID" << delim << "Name" << delim << "Type" << delim << "Icon" << delim << "Version"
            << delim << "Game Path" << delim << "Java Path\n";
  for (const ProfileData& profile_data : profile_datas) {
    std::cout << profile_data.id << delim;
    std::cout << profile_data.name_opt.value_or("") << delim;
    std::cout << profile_data.type_opt.value_or("") << delim;
    std::cout << profile_data.icon_opt.value_or("") << delim;
    std::cout << profile_data.version_opt.value_or("") << delim;
    std::cout << profile_data.game_path_opt.value_or(fs::path()).string() << delim;
    std::cout << profile_data.java_path_opt.value_or(fs::path()).string() << "\n";
  }
}

}  // namespace

}  // namespace tl
