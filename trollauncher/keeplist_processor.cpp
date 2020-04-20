// Copyright (c) 2020 Tim Perkins

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

#include "trollauncher/keeplist_processor.hpp"

#include <regex>

namespace tl {

namespace {

namespace fs = std::filesystem;

// TODO With Structurize it might be good to allow updating with new schematics, but make sure not
// to delete or overwrite user schematics? The trouble is, how would you know?

static const std::vector<std::regex> default_keep_regexes = {
    // Minecraft data
    std::regex("^crash-reports/"),
    std::regex("^logs/"),
    std::regex("^resourcepacks/"),
    std::regex("^saves/"),
    std::regex("^screenshots/"),
    std::regex("^hotbar.nbt"),
    std::regex("^options.txt"),
    std::regex("^servers.dat"),
    std::regex("^usercache.json"),
    std::regex("^usernamecache.json"),
    // Optifine
    std::regex("^shaderpacks/"),
    std::regex("^optionsof.txt"),
    // Reauth data
    std::regex("^reauth.toml"),
    // Xaero map data
    std::regex("^XaeroWaypoints/"),
    std::regex("^XaeroWorldMap/"),
    // Structurize
    std::regex("^structurize/"),
    // Anything Git related
    std::regex("^.git/"),
    std::regex("^.gitignore"),
    std::regex("^.gitmodules"),
};

}  // namespace

struct KeeplistProcessor::Data_ {
  std::vector<std::regex> keep_regexes;
};

KeeplistProcessor::KeeplistProcessor() : data_(std::make_unique<KeeplistProcessor::Data_>())
{
  // Do nothing
}

KeeplistProcessor::Ptr KeeplistProcessor::Create(const fs::path&, std::error_code*)
{
  // TODO Someday we should be able to load a custom keeplist
  throw std::runtime_error("Not implemented");
}

KeeplistProcessor::Ptr KeeplistProcessor::CreateDefault()
{
  auto klp_ptr = Ptr(new KeeplistProcessor());
  klp_ptr->data_->keep_regexes = default_keep_regexes;
  return klp_ptr;
}

bool KeeplistProcessor::IsOverwritePath(const fs::path& path) const
{
  for (const std::regex& keep_regex : data_->keep_regexes) {
    if (std::regex_search(path.generic_string(), keep_regex)) {
      return false;
    }
  }
  return true;
}

std::vector<fs::path> KeeplistProcessor::FilterOverwritePaths(
    const std::vector<fs::path>& paths) const
{
  std::vector<fs::path> filtered_paths;
  for (const fs::path& path : paths) {
    if (IsOverwritePath(path)) {
      filtered_paths.push_back(path);
    }
  }
  return filtered_paths;
}

}  // namespace tl
