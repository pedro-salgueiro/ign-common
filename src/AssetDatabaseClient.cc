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

#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <string>
#include <vector>

#include "ignition/common/AssetDatabaseClient.hh"

using namespace ignition;
using namespace common;

namespace ignition
{
  namespace common
  {
    /// \internal
    /// \brief Private data for AssetDatabaseClient class.
    class AssetDatabaseClientPrivate
    {
      /// \brief Constructor.
      public: explicit AssetDatabaseClientPrivate()
      {
      }

      /// \brief Destructor.
      public: virtual ~AssetDatabaseClientPrivate() = default;
    };
  }
}

/////////////////////////////////////////////////
size_t get_assets_cb(void *_buffer, size_t _size, size_t _nmemb, void *_userp)
{
  std::string *str = static_cast<std::string*>(_userp);
  _size *= _nmemb;

  // Append the new character data to the string
  str->append(static_cast<const char*>(_buffer), _size);
  return _size;
}

/////////////////////////////////////////////////
AssetDatabaseClient::AssetDatabaseClient()
{
  this->dataPtr.reset(new AssetDatabaseClientPrivate());
}

/////////////////////////////////////////////////
AssetDatabaseClient::~AssetDatabaseClient()
{
}

/////////////////////////////////////////////////
bool AssetDatabaseClient::LoadConfig(const std::string &_configPath)
{

}

/////////////////////////////////////////////////
std::vector<std::string> AssetDatabaseClient::Folder() const
{
  std::string jsonString;
  std::vector<std::string> res;
  std::string uri = "http://localhost:8080/api/v1/folder?parentType=folder"
                    "&parentId=58ffb6be96a00f0cd30890a5&sort=lowerName"
                    "&sortdir=1";

  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_assets_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsonString);
  CURLcode success = curl_easy_perform(curl);
  if (success != CURLE_OK)
    std::cerr << "Unable to connect to asset database\n";

  curl_easy_cleanup(curl);

  Json::Reader reader;
  Json::Value obj;
  reader.parse(jsonString, obj);

  for (const auto &asset : obj)
    res.push_back(asset["name"].asString());

  return res;
}