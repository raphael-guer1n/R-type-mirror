#pragma once

#include <asio.hpp>

namespace engine::net
{

    class IoContextImpl
    {
    public:
        asio::io_context ctx;
    };

} // namespace engine::net
