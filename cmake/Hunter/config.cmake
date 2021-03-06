#[[
   Copyright 2021 The Silkoroutine Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
]]

hunter_config(Boost VERSION 1.72.0-p1)    # <-- Highest available on hunter
hunter_config(abseil VERSION 20200923.2)  # <-- Required for C++20
hunter_config(
  Catch
  VERSION 2.13.4
  URL https://github.com/catchorg/Catch2/archive/v2.13.4.tar.gz
  SHA1 b8417c5c87ab385c9f56576aefbcc098fb923e57
)
hunter_config(Protobuf VERSION 3.12.4-p1)  # <-- Last working version (greater seems to have networking issues)
hunter_config(gRPC VERSION 1.31.0-p0)      # <-- Last working version (greater seems to have networking issues)
