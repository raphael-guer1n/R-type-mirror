#pragma once

#include <memory>

namespace engine::net
{

    class IoContextImpl;

    class IoContext
    {
    public:
        IoContext();
        ~IoContext();
        IoContext(IoContext &&) noexcept;
        IoContext &operator=(IoContext &&) noexcept;
        IoContext(const IoContext &) = delete;
        IoContext &operator=(const IoContext &) = delete;

        // Run/poll the underlying context
        void run();
        void poll();
        void stop();

        // Internal: access to impl for engine internals
        IoContextImpl *impl();
        const IoContextImpl *impl() const;

        // Internal: return underlying context as opaque pointer
        void *native_handle();
        const void *native_handle() const;

    private:
        std::unique_ptr<IoContextImpl> _impl;
    };

} // namespace engine::net
