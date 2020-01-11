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

#ifndef TROLLAUNCHER_UTILS_HPP_
#define TROLLAUNCHER_UTILS_HPP_

#include <optional>
#include <string>
#include <vector>

namespace tl {

// FUN FACT: The function is called "GetEnvironmentVar" because on Windows, apparently, some genius
// decided to make "GetEnvironmentVariable" a macro for "GetEnvironmentVariableA". Yikes.

std::optional<std::string> GetEnvironmentVar(const std::string& name);
std::string GetRandomId();
std::string GetRandomName();
std::vector<std::string> GetDefaultLauncherIcons();

}  // namespace tl

#endif  // TROLLAUNCHER_UTILS_HPP_
