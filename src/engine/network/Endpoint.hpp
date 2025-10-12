#pragma once

#include <string>

namespace engine::net
{

    // Simple value type representing a UDP endpoint without exposing Asio.
    struct Endpoint
    {
        std::string address; // IPv4/IPv6 textual address
        unsigned short port{};
        bool operator==(const Endpoint &other) const
        {
            return address == other.address && port == other.port;
        }
    };

    inline Endpoint make_endpoint(const std::string &addr, unsigned short p)
    {
        return Endpoint{addr, p};
    }

} // namespace engine::net
