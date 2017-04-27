/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <algorithm>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <string>

#include "ignition/common/ParserINI.hh"
#include "ignition/common/StringUtils.hh"

using namespace ignition;
using namespace common;

namespace ignition
{
  namespace common
  {
    /// \internal
    /// \brief Private data for ParserINI class.
    class ParserINIPrivate
    {
      /// \brief Constructor.
      public: explicit ParserINIPrivate(const std::string &_filePath)
        : filePath(_filePath)
      {
      }

      /// \brief Destructor.
      public: virtual ~ParserINIPrivate() = default;

      /// \brief.
      public: std::string filePath;

      /// \brief
      public: std::map<std::string, std::map<std::string, std::string>> data;
    };
  }
}

/////////////////////////////////////////////////
ParserINI::ParserINI(const std::string &_filePath)
{
  this->dataPtr.reset(new ParserINIPrivate(_filePath));
  if (!this->Parse())
    this->dataPtr->data.clear();
}

/////////////////////////////////////////////////
ParserINI::~ParserINI()
{
}

/////////////////////////////////////////////////
bool ParserINI::Parse()
{
  std::ifstream file(this->dataPtr->filePath);

  // Sanity check: Check that the file existed.
  if (!file)
  {
    std::cerr << "Unable to open INI file [" << this->dataPtr->filePath
              << "]" << std::endl;
    return false;
  }

  // Load the content.
  std::stringstream fileContent;
  fileContent << file.rdbuf();
  file.close();

  std::string line;
  std::string category = "";
  size_t lineNum = 0;

  while (std::getline(fileContent, line))
  {
    ++lineNum;

    // Remove leading and trailing whitespaces.
    while (isspace(line.front()))
      line.erase(0, 1);

    while (isspace(line.back()))
      line.pop_back();

    // Ignore comments.
    if (StartsWith(line, ";"))
      continue;

    // Ignore a blank line.
    if (line.empty())
      continue;

    // Replace all spaces with ' '.
    std::replace_if(line.begin(), line.end(), ::isspace, ' ');

    // Check whether we are parsing a category or a key/value.
    auto startCategoryDelim = line.find("[");
    if (startCategoryDelim != std::string::npos)
    {
      auto endCategoryDelim = line.find("]");

      // Sanity check: We should have a "]" if we found a "[" before.
      if (endCategoryDelim == std::string::npos)
      {
        std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                  << "Error parsing category: Unable to find \"]\""
                  << std::endl;
        return false;
      }

      category = line.substr(startCategoryDelim + 1, endCategoryDelim - 1);

      // Remove leading and trailing whitespaces.
      while (isspace(category.front()))
        category.erase(0, 1);

      while (isspace(category.back()))
        category.pop_back();

      // Sanity check: The category name shouldn't be empty.
      if (category.empty())
      {
        std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                  << "Error parsing category: Empty category" << std::endl;
        return false;
      }

      // Sanity check: The category shouldn't exist.
      if (this->dataPtr->data.find(category) != this->dataPtr->data.end())
      {
        std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                  << "Error parsing category: Repeated category ["
                  << category << "]" << std::endl;
        return false;
      }

      // Save the category.
      this->dataPtr->data[category] = std::map<std::string, std::string>();
    }
    else
    {
      auto keyValueDelim = line.find("=");

      // Sanity check: The "=" delimiter should exist.
      if (keyValueDelim == std::string::npos)
      {
        std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                  << "Error parsing key/value: Unable to find \"=\" character"
                  << std::endl;
        return false;
      }

      auto keyValue = Split(line, '=');

      // Sanity check: The resulted vector should only have two elements
      // (key and value).
      if (keyValue.size() != 2)
      {
        std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                  << "Error parsing key/value: Found more than one \"=\" "
                  << "character" << std::endl;
        return false;
      }

      auto key = keyValue.at(0);
      // Remove leading and trailing whitespaces.
      while (isspace(key.front()))
        key.erase(0, 1);

      while (isspace(key.back()))
        key.pop_back();

      auto value = keyValue.at(1);
      // Remove leading and trailing whitespaces.
      while (isspace(value.front()))
        value.erase(0, 1);

      while (isspace(value.back()))
        value.pop_back();

      // Sanity check: The key shouldn't exist.
      if (this->dataPtr->data.find(category) != this->dataPtr->data.end())
      {
        if (this->dataPtr->data[category].find(key) !=
             this->dataPtr->data[category].end())
        {
          std::cerr << "[Line " << lineNum << "][" << line << "]\n"
                    << "Error parsing key/value: Repeated key [" << key << "]"
                    << std::endl;
          return false;
        }
      }

      // Save the key/value.
      this->dataPtr->data[category][key] = value;
    }
  }

  return true;
}

///////////////////////////////////////////////////
std::string ParserINI::ToString() const
{
  std::string res;
  for (auto const &categoryPair : this->dataPtr->data)
  {
    res += "[" + categoryPair.first + "]\n";
    auto &kvs = categoryPair.second;
    for (auto const &kvPair : kvs)
      res += kvPair.first + "=" + kvPair.second + "\n";
  }
  return res;
}
