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
#ifndef IGNITION_COMMON_ASSETDATABASECLIENT_HH_
#define IGNITION_COMMON_ASSETDATABASECLIENT_HH_

#include <memory>
#include <string>
#include <vector>

#include "ignition/common/SingletonT.hh"
#include "ignition/common/System.hh"

namespace ignition
{
  namespace common
  {
    /// \brief Forward declare private data class.
    class AssetDatabaseClientPrivate;

    /// \class AssetDatabaseClient AssetDatabaseClient.hh common/common.hh
    /// \brief Connects to asset database, and has utility functions to find
    /// assets.
    class IGNITION_COMMON_VISIBLE AssetDatabaseClient
      : public SingletonT<AssetDatabaseClient>
    {
      /// \brief ToDo.
      public: AssetDatabaseClient();

      /// \brief ToDo.
      public: virtual ~AssetDatabaseClient();

      /// \brief ToDo.
      public: bool LoadConfig(const std::string &_configPath);

      public: std::vector<std::string> Folder() const;

      /// \brief Private data.
      private: std::unique_ptr<AssetDatabaseClientPrivate> dataPtr;
    };
  }
}

#endif
