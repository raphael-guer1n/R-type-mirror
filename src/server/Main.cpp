#include "server/Server.hpp"

int main() {
    engine::net::IoContext io;
    server s(io, 4242);
    s.run();
    return 0;
}
