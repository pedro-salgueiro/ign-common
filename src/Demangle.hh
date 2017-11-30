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

#ifndef IGNITION_COMMON_DEMANGLE_HH_
#define IGNITION_COMMON_DEMANGLE_HH_

#include <string>

#if defined __GNUC__ or defined __clang__
// This header is used for name demangling on GCC and Clang
#include <cxxabi.h>
#endif

#include <ignition/common/Console.hh>

namespace ignition
{
  namespace common
  {
    /////////////////////////////////////////////////
    inline std::string Demangle(const std::string &_name)
    {
#if defined __GNUC__ or defined __clang__
      int status;
      char *demangled_cstr = abi::__cxa_demangle(
            _name.c_str(), nullptr, nullptr, &status);

      if (0 != status)
      {
        ignerr << "[Demangle] Failed to demangle the symbol name [" << _name
               << "]. Error code: " << status << "\n";
        return _name;
      }

      const std::string demangled(demangled_cstr);
      free(demangled_cstr);

      return demangled;
#else
      // MSVC does not require any demangling. Any other compilers besides GCC
      // or clang may be using an unknown ABI, so we won't be able to demangle
      // them anyway.
      return _name;
#endif
    }
  }
}

#endif
