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

#ifndef SILKOROUTINE_KV_ASYNC_CLOSE_HPP
#define SILKOROUTINE_KV_ASYNC_CLOSE_HPP

#include <asio/detail/config.hpp>
#include <asio/detail/bind_handler.hpp>
#include <asio/detail/fenced_block.hpp>
#include <asio/detail/handler_alloc_helpers.hpp>
#include <asio/detail/handler_work.hpp>
#include <asio/detail/memory.hpp>

#include <silkoroutine/ethdb/kv/async_operation.hpp>

namespace silkoroutine::ethdb::kv {

template <typename Handler, typename IoExecutor>
class async_close : public async_operation<void, uint32_t>
{
public:
    ASIO_DEFINE_HANDLER_PTR(async_close);

    async_close(Handler& h, const IoExecutor& io_ex)
    : async_operation(&async_close::do_complete), handler_(ASIO_MOVE_CAST(Handler)(h)), work_(handler_, io_ex)
    {}

    static void do_complete(void* owner, async_operation* base, uint32_t cursor_id=0) {
        // Take ownership of the handler object.
        async_close* h{static_cast<async_close*>(base)};
        ptr p = {asio::detail::addressof(h->handler_), h, h};

        ASIO_HANDLER_COMPLETION((*h));

        // Take ownership of the operation's outstanding work.
        asio::detail::handler_work<Handler, IoExecutor> w(
            ASIO_MOVE_CAST2(asio::detail::handler_work<Handler, IoExecutor>)(h->work_)
        );

        // Make a copy of the handler so that the memory can be deallocated before
        // the upcall is made. Even if we're not about to make an upcall, a
        // sub-object of the handler may be the true owner of the memory associated
        // with the handler. Consequently, a local copy of the handler is required
        // to ensure that any owning sub-object remains valid until after we have
        // deallocated the memory here.
        asio::detail::binder1<Handler, uint32_t> handler{h->handler_, cursor_id};
        p.h = asio::detail::addressof(handler.handler_);
        p.reset();

        // Make the upcall if required.
        if (owner) {
            asio::detail::fenced_block b(asio::detail::fenced_block::half);
            ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
            w.complete(handler, handler.handler_);
            ASIO_HANDLER_INVOCATION_END;
        }
    }

private:
    Handler handler_;
    asio::detail::handler_work<Handler, IoExecutor> work_;
};

} // namespace silkoroutine::ethdb::kv

#endif // SILKOROUTINE_KV_ASYNC_CLOSE_HPP
