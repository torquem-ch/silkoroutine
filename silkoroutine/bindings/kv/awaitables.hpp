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

#ifndef SILKOROUTINE_BINDINGS_KV_AWAITABLES_HPP_
#define SILKOROUTINE_BINDINGS_KV_AWAITABLES_HPP_

#include <silkoroutine/config.hpp>

#include <functional>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

#include <asio/async_result.hpp>
#include <asio/detail/non_const_lvalue.hpp>
#include <asio/error.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <grpcpp/grpcpp.h>

#include <silkoroutine/common/util.hpp>
#include <silkoroutine/bindings/kv/async_close_cursor.hpp>
#include <silkoroutine/bindings/kv/async_close.hpp>
#include <silkoroutine/bindings/kv/async_next.hpp>
#include <silkoroutine/bindings/kv/async_open_cursor.hpp>
#include <silkoroutine/bindings/kv/async_seek.hpp>
#include <silkoroutine/bindings/kv/grpc_client_reactor.hpp>
#include <silkoroutine/bindings/kv/generated/kv.grpc.pb.h>

namespace silkoroutine::bindings::kv {

template<typename Executor>
struct GrpcAsioAwaitable;

template<typename Executor>
class initiate_async_open_cursor {
public:
    typedef Executor executor_type;

    explicit initiate_async_open_cursor(GrpcAsioAwaitable<Executor>* self, const std::string& table_name)
    : self_(self), table_name_(table_name) {}

    executor_type get_executor() const noexcept { return self_->get_executor(); }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler) {
        ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

        asio::detail::non_const_lvalue<WaitHandler> handler2(handler);
        typedef async_open_cursor<WaitHandler, Executor> op;
        typename op::ptr p = {asio::detail::addressof(handler2.value), op::ptr::allocate(handler2.value), 0};
        wrapper_ = new op(handler2.value, self_->context_.get_executor());

        auto open_message = remote::Cursor{};
        open_message.set_op(remote::Op::OPEN);
        open_message.set_bucketname(table_name_);
        self_->reactor_.write_start(&open_message, [this](bool ok) {
            if (!ok) {
                throw std::system_error{std::make_error_code(std::errc::io_error), "write failed in OPEN_CURSOR"};
            }
            self_->reactor_.read_start([this](bool ok, remote::Pair open_pair) {
                auto cursor_id = open_pair.cursorid();

                typedef async_open_cursor<WaitHandler, Executor> op;
                auto open_cursor_op = static_cast<op*>(wrapper_);

                // Make the io_context thread execute the operation completion
                self_->context_.post([this, ok, open_cursor_op, cursor_id]() {
                    if (!ok) {
                        throw std::system_error{std::make_error_code(std::errc::io_error), "read failed in OPEN_CURSOR"};
                    }
                    open_cursor_op->complete(this, cursor_id);
                });
            });
        });
    }

private:
    GrpcAsioAwaitable<Executor>* self_;
    const std::string& table_name_;
    void* wrapper_;
};

template<typename Executor>
class initiate_async_seek {
public:
    typedef Executor executor_type;

    explicit initiate_async_seek(GrpcAsioAwaitable<Executor>* self, uint32_t cursor_id, const Bytes& seek_key_bytes)
    : self_(self), cursor_id_(cursor_id), seek_key_bytes_(std::move(seek_key_bytes)) {}

    executor_type get_executor() const noexcept { return self_->get_executor(); }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler) {
        ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

        asio::detail::non_const_lvalue<WaitHandler> handler2(handler);
        typedef async_seek<WaitHandler, Executor> op;
        typename op::ptr p = {asio::detail::addressof(handler2.value), op::ptr::allocate(handler2.value), 0};
        wrapper_ = new op(handler2.value, self_->context_.get_executor());

        auto seek_message = remote::Cursor{};
        seek_message.set_op(remote::Op::SEEK);
        seek_message.set_cursor(cursor_id_);
        seek_message.set_k(seek_key_bytes_.c_str(), seek_key_bytes_.length());
        self_->reactor_.write_start(&seek_message, [this](bool ok) {
            if (!ok) {
                throw std::system_error{std::make_error_code(std::errc::io_error), "write failed in SEEK"};
            }
            self_->reactor_.read_start([this](bool ok, remote::Pair seek_pair) {
                typedef async_seek<WaitHandler, Executor> op;
                auto seek_op = static_cast<op*>(wrapper_);

                // Make the io_context thread execute the operation completion
                self_->context_.post([this, ok, seek_op, seek_pair]() {
                    if (!ok) {
                        throw std::system_error{std::make_error_code(std::errc::io_error), "read failed in SEEK"};
                    }
                    seek_op->complete(this, seek_pair);
                });
            });
        });
    }

private:
    GrpcAsioAwaitable<Executor>* self_;
    uint32_t cursor_id_;
    const Bytes seek_key_bytes_;
    void* wrapper_;
};

template<typename Executor>
class initiate_async_next {
public:
    typedef Executor executor_type;

    explicit initiate_async_next(GrpcAsioAwaitable<Executor>* self, uint32_t cursor_id)
    : self_(self), cursor_id_(cursor_id) {}

    executor_type get_executor() const noexcept { return self_->get_executor(); }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler) {
        ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

        asio::detail::non_const_lvalue<WaitHandler> handler2(handler);
        typedef async_next<WaitHandler, Executor> op;
        typename op::ptr p = {asio::detail::addressof(handler2.value), op::ptr::allocate(handler2.value), 0};
        wrapper_ = new op(handler2.value, self_->context_.get_executor());

        auto next_message = remote::Cursor{};
        next_message.set_op(remote::Op::NEXT);
        next_message.set_cursor(cursor_id_);
        self_->reactor_.write_start(&next_message, [this](bool ok) {
            if (!ok) {
                throw std::system_error{std::make_error_code(std::errc::io_error), "write failed in NEXT"};
            }
            self_->reactor_.read_start([this](bool ok, remote::Pair next_pair) {
                typedef async_next<WaitHandler, Executor> op;
                auto next_op = static_cast<op*>(wrapper_);

                // Make the io_context thread execute the operation completion
                self_->context_.post([this, ok, next_op, next_pair]() {
                    if (!ok) {
                        throw std::system_error{std::make_error_code(std::errc::io_error), "read failed in NEXT"};
                    }
                    next_op->complete(this, next_pair);
                });
            });
        });
    }

private:
    GrpcAsioAwaitable<Executor>* self_;
    uint32_t cursor_id_;
    const Bytes next_key_bytes_;
    void* wrapper_;
};

template<typename Executor>
class initiate_async_close_cursor {
public:
    typedef Executor executor_type;

    explicit initiate_async_close_cursor(GrpcAsioAwaitable<Executor>* self, uint32_t cursor_id)
    : self_(self), cursor_id_(cursor_id) {}

    executor_type get_executor() const noexcept { return self_->get_executor(); }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler) {
        ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

        asio::detail::non_const_lvalue<WaitHandler> handler2(handler);
        typedef async_close_cursor<WaitHandler, Executor> op;
        typename op::ptr p = {asio::detail::addressof(handler2.value), op::ptr::allocate(handler2.value), 0};
        wrapper_ = new op(handler2.value, self_->context_.get_executor());

        auto close_message = remote::Cursor{};
        close_message.set_op(remote::Op::CLOSE);
        close_message.set_cursor(cursor_id_);
        self_->reactor_.write_start(&close_message, [this](bool ok) {
            if (!ok) {
                throw std::system_error{std::make_error_code(std::errc::io_error), "write failed in CLOSE_CURSOR"};
            }
            self_->reactor_.read_start([this](bool ok, remote::Pair close_pair) {
                auto cursor_id = close_pair.cursorid();

                typedef async_close_cursor<WaitHandler, Executor> op;
                auto close_cursor_op = static_cast<op*>(wrapper_);

                // Make the io_context thread execute the operation completion
                self_->context_.post([this, ok, close_cursor_op, cursor_id]() {
                    if (!ok) {
                        throw std::system_error{std::make_error_code(std::errc::io_error), "read failed in CLOSE_CURSOR"};
                    }
                    close_cursor_op->complete(this, cursor_id);
                });
            });
        });
    }

private:
    GrpcAsioAwaitable<Executor>* self_;
    uint32_t cursor_id_;
    void* wrapper_;
};

template<typename Executor>
class initiate_async_close {
public:
    typedef Executor executor_type;

    explicit initiate_async_close(GrpcAsioAwaitable<Executor>* self)
    : self_(self) {}

    executor_type get_executor() const noexcept { return self_->get_executor(); }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler) {
        ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

        asio::detail::non_const_lvalue<WaitHandler> handler2(handler);
        typedef async_close<WaitHandler, Executor> op;
        typename op::ptr p = {asio::detail::addressof(handler2.value), op::ptr::allocate(handler2.value), 0};
        wrapper_ = new op(handler2.value, self_->context_.get_executor());

        self_->reactor_.close_start([this](bool ok) {
            typedef async_close<WaitHandler, Executor> op;
            auto close_op = static_cast<op*>(wrapper_);

            // Make the io_context thread execute the operation completion
            self_->context_.post([this, ok, close_op]() {
                if (!ok) {
                    throw std::system_error{std::make_error_code(std::errc::io_error), "gRPC failed in CLOSE"};
                }
                close_op->complete(this, 0);
            });
        });
    }

private:
    GrpcAsioAwaitable<Executor>* self_;
    void* wrapper_;
};

template<typename Executor>
struct GrpcAsioAwaitable {
    typedef Executor executor_type;

    explicit GrpcAsioAwaitable(asio::io_context& context, GrpcClientReactor& reactor)
    : context_(context), reactor_(reactor) {}

    template<typename WaitHandler>
    auto async_open_cursor(const std::string& table_name, WaitHandler&& handler) {
        return asio::async_initiate<WaitHandler, void(uint32_t)>(initiate_async_open_cursor{this, table_name}, handler);
    }

    template<typename WaitHandler>
    auto async_seek(uint32_t cursor_id, const Bytes& seek_key_bytes, WaitHandler&& handler) {
        return asio::async_initiate<WaitHandler, void(remote::Pair)>(initiate_async_seek{this, cursor_id, seek_key_bytes}, handler);
    }

    template<typename WaitHandler>
    auto async_next(uint32_t cursor_id, WaitHandler&& handler) {
        return asio::async_initiate<WaitHandler, void(remote::Pair)>(initiate_async_next{this, cursor_id}, handler);
    }

    template<typename WaitHandler>
    auto async_close_cursor(uint32_t cursor_id, WaitHandler&& handler) {
        return asio::async_initiate<WaitHandler, void(uint32_t)>(initiate_async_close_cursor{this, cursor_id}, handler);
    }

    template<typename WaitHandler>
    auto async_close(WaitHandler&& handler) {
        return asio::async_initiate<WaitHandler, void(uint32_t)>(initiate_async_close{this}, handler);
    }

    asio::io_context& context_;
    GrpcClientReactor& reactor_;
};

} // namespace silkoroutine::bindings::kv

#endif // SILKOROUTINE_BINDINGS_KV_AWAITABLES_HPP_
