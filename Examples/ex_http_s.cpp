/*
* Copyright 2015 Valentin Milea
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifdef HAVE_BOOST_CONTEXT

#include "Common.h"
#include "util/AsioHttp.h"
#include <CppAsync/AsioWrappers.h>
#include <CppAsync/StackfulAsync.h>
#include <fstream>

namespace asio = boost::asio;
using asio::ip::tcp;

static asio::io_service sIo;

static ut::Task<void> asyncHttpDownload(asio::streambuf& outBuf, std::string host, std::string path)
{
    return ut::stackful::startAsync([&outBuf, host, path]() {
        struct Context
        {
            tcp::socket socket;

            Context()
                : socket(sIo) { }
        };
        auto ctx = ut::makeContext<Context>();

        auto connectTask = ut::asyncResolveAndConnect(ctx->socket, ctx,
            asio::ip::tcp::resolver::query(host, "http"));
        ut::stackful::await_(connectTask);

        auto downloadTask = util::asyncHttpGet(ctx->socket, outBuf, ctx, host, path, false);
        ut::stackful::await_(downloadTask);
    });
}

void ex_http_s()
{
    asio::streambuf buf;

    auto task = asyncHttpDownload(buf,
        "www.google.com",
        "/images/branding/googlelogo/2x/googlelogo_color_272x92dp.png");

    sIo.run();

    assert(task.isReady());

    try {
        task.get();

        printf("saving download.png (%d bytes)...\n", (int) buf.size());

        std::ofstream fout("download.png", std::ios::out | std::ios::binary);
        fout << &buf;
    } catch (std::exception& e) {
        printf("HTTP download failed: %s\n", e.what());
        return;
    }
}

#endif // HAVE_BOOST_CONTEXT