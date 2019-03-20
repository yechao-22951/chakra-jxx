#pragma once
#include <asio.hpp>
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include "js.primitive.h"
#include "jxx.class.h"
#include "jxx.runtime.h"
#include <functional>

namespace js {

    using namespace asio::ip;

    asio::io_context& current_io_context() {
        auto jsrt = JxxGetCurrentRuntime();
        CXX_EXCEPTION_IF(JsErrorInvalidContext, !jsrt);
        return jsrt->io_context();
    }

    class _tcp_socket_t : public tcp::socket {
    public:
        _tcp_socket_t() : tcp::socket(current_io_context()) {}
        _tcp_socket_t(tcp::socket&& r) : tcp::socket(std::move(r)) {};
    };
    using tcp_socket_t = JxxOf<_tcp_socket_t>;

    class _tcp_acceptor_t : public tcp::acceptor {
    public:
        _tcp_acceptor_t() : tcp::acceptor(current_io_context()) {}
    };
    using tcp_acceptor_t = JxxOf<_tcp_acceptor_t>;

    _as_the<_Object> CreateAcceptor(
        _as_the<_String> addr,
        _as_the<_String> port,
        _as_the<_Object | _Optional> options)
    {
        JxxObjectPtr<tcp_acceptor_t> acceptor_ = new tcp_acceptor_t();
        if (!acceptor_) return JS_INVALID_REFERENCE;
        std::string remote_ = get_as_<std::string>(addr);
        std::string port_ = get_as_<std::string>(port);
        tcp::resolver resolver(current_io_context());
        tcp::endpoint endpoint = *resolver.resolve(remote_, port_).begin();
        acceptor_->open(endpoint.protocol());
        acceptor_->set_option(tcp::acceptor::reuse_address(true));
        acceptor_->bind(endpoint);
        return ExtObject::Create(acceptor_, nullptr);
    }

    bool do_accept(ExtObject acceptor_) {
        Durable<ExtObject> jsAcceptor(acceptor_);
        JxxObjectPtr<tcp_acceptor_t> cxxAcceptor = jsAcceptor->TryGetAs<tcp_acceptor_t>();
        if (!cxxAcceptor) return false;
        cxxAcceptor->async_accept(
            [cxxAcceptor, jsAcceptor](std::error_code ec, tcp::socket socket) mutable
            {
                if (!cxxAcceptor->is_open())
                    return;
                if (ec) return;
                Function onConnection = jsAcceptor->GetProperty(JxxGetPropertyId("onConnection"));
                if (!onConnection)
                    return;
                auto client = ExtObject::As<tcp_socket_t>::New(nullptr, std::move(socket));
                if (!client)
                    return;
                onConnection.Call(jsAcceptor.get(), client.get());
                do_accept(jsAcceptor);
            });
        return true;
    };

    _as_the<_Boolean> Listen(_as_the<_Object> acceptor) {
        ExtObject jsAcceptor(acceptor);
        Function onConnection = jsAcceptor["onConnection"];
        if (!onConnection) return just_is_(false);
        auto cxxAcceptor = jsAcceptor.TryGetAs<tcp_acceptor_t>();
        if (!cxxAcceptor) return just_is_(false);
        cxxAcceptor->listen();
        if (!do_accept(jsAcceptor)) return just_is_(false);
        return just_is_(true);
    }

    _as_the<_Boolean> AsyncRead(_as_the<_Object> socket, _as_the<_BufferLike> buffer)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return just_is_(false);
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        if (!cxxSocket) return just_is_(false);
        Durable<value_ref_t> jsBuffer = buffer;
        content_t cxxBuffer = GetContent(jsBuffer.get());
        cxxSocket->async_read_some(asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer](std::error_code ec, size_t read)
            {
                Function callback = (value_ref_t)jsSocket->GetProperty("onData");
                if (!callback) return;
                callback.Call(
                    jsSocket.get(),
                    just_is_(ec.value()),
                    just_is_(read),
                    jsBuffer.get()
                );
            });
        return just_is_(true);
    };

    _as_the<_Boolean> AsyncReadEx(_as_the<_Object> socket, _as_the<_Number> bufferSize)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return just_is_(false);
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        if (!cxxSocket) return just_is_(false);
        Durable<ArrayBuffer> jsBuffer = ArrayBuffer::Alloc(get_as_<int>(bufferSize));
        if (!jsBuffer) return just_is_(false);
        content_t cxxBuffer = GetContent(jsBuffer.get());
        cxxSocket->async_read_some(
            asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer, bufferSize](std::error_code ec, size_t read)
            {
                Function callback = jsSocket->GetProperty("onData");
                if (!callback) return;
                callback.Call(
                    jsSocket.get(),
                    just_is_(ec.value()),
                    just_is_(read),
                    jsBuffer.get()
                );
                if (!ec)
                    AsyncReadEx(jsSocket.get(), bufferSize);
            });

        return just_is_(true);
    };

    _as_the<_Boolean> AsyncWrite(_as_the<_Object> socket, _as_the<_BufferLike> buffer)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return just_is_(false);
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        content_t cxxBuffer = GetContent(buffer);
        Durable<value_ref_t> jsBuffer(buffer);
        cxxSocket->async_send(
            asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer](std::error_code ec, size_t written) mutable {
                Function onWritten = jsSocket->GetProperty("onWritten");
                if (!onWritten) return;
                onWritten.Call(
                    jsSocket.get(),
                    just_is_(ec.value()),
                    just_is_(written),
                    jsBuffer.get()
                );
            });
        return just_is_(true);
    }

}; // namespace js