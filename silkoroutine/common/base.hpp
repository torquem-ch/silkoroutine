/*
   Copyright 2020 The Silkoroutine Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef SILKOROUTINE_COMMON_BASE_HPP_
#define SILKOROUTINE_COMMON_BASE_HPP_

#include <string>
#include <string_view>

namespace silkoroutine {

using Bytes = std::basic_string<uint8_t>;
using ByteView = std::basic_string_view<uint8_t>;

struct KeyValue {
    Bytes key;
    Bytes value;
};

} // namespace silkoroutine

#endif  // SILKOROUTINE_COMMON_BASE_HPP_
