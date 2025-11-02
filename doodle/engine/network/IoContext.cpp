#include "engine/network/IoContext.hpp"

#include "engine/network/detail/IoContextInternal.hpp"

namespace engine::net
{

    IoContext::IoContext() : _impl(std::make_unique<IoContextImpl>()) {}
    IoContext::~IoContext() = default;
    IoContext::IoContext(IoContext &&) noexcept = default;
    IoContext &IoContext::operator=(IoContext &&) noexcept = default;

    void IoContext::run() { _impl->ctx.run(); }
    void IoContext::poll() { _impl->ctx.poll(); }
    void IoContext::stop() { _impl->ctx.stop(); }

    IoContextImpl *IoContext::impl() { return _impl.get(); }
    const IoContextImpl *IoContext::impl() const { return _impl.get(); }

    void *IoContext::native_handle() { return static_cast<void *>(&_impl->ctx); }
    const void *IoContext::native_handle() const { return static_cast<const void *>(&_impl->ctx); }

} // namespace engine::net
