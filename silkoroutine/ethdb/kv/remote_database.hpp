/*
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
*/

#ifndef SILKOROUTINE_ETHDB_KV_REMOTE_DATABASE_HPP_
#define SILKOROUTINE_ETHDB_KV_REMOTE_DATABASE_HPP_

#include <memory>

#include <asio/io_context.hpp>
#include <grpcpp/grpcpp.h>

#include <silkoroutine/ethdb/kv/database.hpp>
#include <silkoroutine/ethdb/kv/remote_transaction.hpp>

namespace silkoroutine::ethdb::kv {

class RemoteDatabase :public Database {
public:
    RemoteDatabase(asio::io_context& context, std::shared_ptr<grpc::Channel> channel)
    : context_(context), channel_(channel) {}

    RemoteDatabase(const RemoteDatabase&) = delete;
    RemoteDatabase& operator=(const RemoteDatabase&) = delete;

    std::unique_ptr<Transaction> begin() override {
        return std::make_unique<RemoteTransaction>(context_, channel_);
    }

private:
    asio::io_context& context_;
    std::shared_ptr<grpc::Channel> channel_;
};

} // namespace silkoroutine::ethdb::kv

#endif  // SILKOROUTINE_ETHDB_KV_REMOTE_DATABASE_HPP_
