#include "server/Server.hpp"
#include <asio.hpp>

int main() {
    asio::io_context io;
    server s(io, 4242);
    s.run();
    return 0;
}
