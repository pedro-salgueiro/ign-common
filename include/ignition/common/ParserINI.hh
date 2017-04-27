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
#ifndef IGNITION_COMMON_PARSERINI_HH_
#define IGNITION_COMMON_PARSERINI_HH_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ignition/common/System.hh"

namespace ignition
{
  namespace common
  {
    /// \brief Forward declare private data class.
    class ParserINIPrivate;

    /// \brief ToDo.
    class IGNITION_COMMON_VISIBLE ParserINI
    {
      /// \brief ToDo.
      public: ParserINI(const std::string &_filePath);

      /// \brief ToDo.
      public: virtual ~ParserINI();

      /// \brief ToDo.
      private: bool Parse();

      /// \brief ToDo.
      private: std::string ToString() const;

      /// \brief Stream insertion operator.
      /// \param[out] _out The output stream.
      /// \param[in] _id UniqueId to write to the stream.
      public: friend std::ostream &operator<<(std::ostream &_out,
                                              const ParserINI &_parser)
      {
        _out << _parser.ToString();
        return _out;
      }

      /// \brief Private data.
      private: std::unique_ptr<ParserINIPrivate> dataPtr;
    };
  }
}

#endif
