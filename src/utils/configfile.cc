/*
  This file is part of Leela Chess Zero.
  Copyright (C) 2018 The LCZero Authors

  Leela Chess is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Leela Chess is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Leela Chess.  If not, see <http://www.gnu.org/licenses/>.

  Additional permission under GNU GPL version 3 section 7

  If you modify this Program, or any covered work, by linking or
  combining it with NVIDIA Corporation's libraries from the NVIDIA CUDA
  Toolkit and the NVIDIA CUDA Deep Neural Network library (or a
  modified version of those libraries), containing parts covered by the
  terms of the respective license agreement, the licensors of this
  Program grant you additional permission to convey the resulting work.
*/

#include <fstream>
#include <iostream>
#include <sstream>

#include "utils/commandline.h"
#include "utils/configfile.h"
#include "utils/optionsparser.h"
#include "utils/string.h"

namespace lczero {
namespace {
const char* kConfigFileStr = "Configuration file path";
const char* kDefaultConfigFile = "lc0.config";
}  // namespace

std::vector<std::string> ConfigFile::arguments_;

void ConfigFile::PopulateOptions(OptionsParser* options) {
  options->Add<StringOption>(kConfigFileStr, "config", 'c') =
      kDefaultConfigFile;
}

bool ConfigFile::Init(OptionsParser* options) {
  arguments_.clear();

  // Process flags to get the config file parameter.
  if (!options->ProcessAllFlags()) return false;

  // Calculate the relative path of the config file.
  OptionsDict dict = options->GetOptionsDict();
  std::string filename = dict.Get<std::string>(kConfigFileStr);
  filename = CommandLine::BinaryDirectory() + "/" + filename;

  // Parses the file into the arguments_ vector.
  if (!ParseFile(filename, options)) return false;

  return true;
}

bool ConfigFile::ParseFile(const std::string filename, OptionsParser* options) {
  std::ifstream input(filename);

  // Check to see if we are using the default config file or not.
  OptionsDict dict = options->GetOptionsDict();
  bool using_default_config = dict.IsDefault<std::string>(kConfigFileStr);

  if (!input.is_open()) {
    // It is okay if we cannot open the default file since it is normal
    // for it to not exist.
    if (using_default_config) return true;

    std::cerr << "Could not open configuration file: " << filename << std::endl;
    return false;
  }

  std::cerr << "Found configuration file: " << filename << std::endl;

  for (std::string line; getline(input, line);) {
    // Remove all leading and trailing whitespace.
    line = Trim(line);
    // Ignore comments.
    if (line.substr(0, 1) == "#") continue;
    // Skip blank lines.
    if (line.length() == 0) continue;
    // Allow long form arugments that omit '--'.  If omitted, add here.
    if (line.substr(0, 1) != "-" && line.substr(0, 2) != "--") {
      line = "--" + line;
    }
    // Fail now if the argument does not begin with '--'.
    if (line.substr(0, 2) != "--") {
      std::cerr << "Only '--' arguments are supported in the "
                << "configuration file: '" << line << "'." << std::endl;
      return false;
    }
    // Add the line to the arguments list.
    arguments_.emplace_back(line);
  }

  return true;
}

}  // namespace lczero