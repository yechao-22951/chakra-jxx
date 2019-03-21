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

    _as_the<_Number> _RegCreateKey(
        _as_the<_Number> hkey,
        _as_the<_String> subkey,
        _as_the<_Object | _Optional> options)
    {
        HKEY h = VoidPtr<HKEY>(hkey);
        std::string path = GetAs<String>(subkey);
        HKEY out = nullptr;
        CXX_EXCEPTION_IF(ErrorSyscall, RegCreateKeyA(h, path.c_str(), &out));
        return VoidPtr(out);
    }

    nullptr_t _RegClose(_as_the<_Number> hkey)
    {
        HKEY h = VoidPtr<HKEY>(hkey);
        CXX_EXCEPTION_IF(ErrorSyscall, ::RegCloseKey(h));
        return nullptr;
    }

    _as_the<_Number | _Buffer | _String | _Nothing>
        _RegQueryValue(
            _as_the<_Number> hkey,
            _as_the<_String> value,
            _as_the<_Number | _Optional> flags = nullptr)
    {
        HKEY h = VoidPtr<HKEY>(hkey);
        std::string name = GetAs<String>(value);
        DWORD dwType = 0, cbData = 0;
        CXX_EXCEPTION_IF(ErrorSyscall, ::RegQueryValueExA(h, name.c_str(), 0, &dwType, NULL, &cbData));
        if (dwType == REG_DWORD || dwType == REG_DWORD_BIG_ENDIAN) {
            DWORD dwValue = 0;
            CXX_EXCEPTION_IF(ErrorSyscall,
                ::RegQueryValueExA(h, name.c_str(), 0, &dwType, (LPBYTE)& dwValue, &cbData));
            return Just<int>(dwValue);
        }
        if (dwType == REG_QWORD || dwType == REG_QWORD_LITTLE_ENDIAN) {
            uint64_t dwValue = 0;
            CXX_EXCEPTION_IF(ErrorSyscall,
                ::RegQueryValueExA(h, name.c_str(), 0, &dwType, (LPBYTE)& dwValue, &cbData));
            return Just<double>(dwValue);
        }
        if (dwType == REG_BINARY) {
            ArrayBuffer buffer = ArrayBuffer::Alloc(cbData);
            content_t conent = buffer.GetContent();
            CXX_EXCEPTION_IF(ErrorSyscall,
                ::RegQueryValueExA(h, name.c_str(), 0, &dwType, (LPBYTE)conent.data, &cbData));
            return buffer;
        }
        if (dwType == REG_EXPAND_SZ || dwType == REG_SZ) {
            std::string data; data.resize(cbData);
            CXX_EXCEPTION_IF(ErrorSyscall,
                ::RegQueryValueExA(h, name.c_str(), 0, &dwType, (LPBYTE)data.data(), &cbData));
            data.resize(cbData - 1);
            return Just(data);
        }
        if (dwType == REG_MULTI_SZ) {
            std::vector<char> data(cbData);
            CXX_EXCEPTION_IF(ErrorSyscall,
                ::RegQueryValueExA(h, name.c_str(), 0, &dwType, (LPBYTE)data.data(), &cbData));
            std::vector<std::string> strings;
            const char* scan = data.data();
            const char* tail = scan + data.size();
            while (*scan && scan < tail) {
                std::string line = scan;
                size_t len = line.size();
                strings.push_back(std::move(scan));
                scan += (len + 1);
            }
            Array arr = Array::Create(strings.size());
            for (size_t i = 0; i < strings.size(); ++i) {
                arr.SetItem(i, Just(strings[i]));
            }
            return arr;
        }
        return nullptr;
    }

    JsValueRef _RegEnumKey(
        _as_the<_Number> hkey,
        _as_the<_Number> mode,
        Function callback
    ) {
        HKEY h = VoidPtr<HKEY>(hkey);
        int mode_ = GetAs<int>(mode);
        DWORD cSubKeys, cbMaxSubKeyLen, cValues, cMaxValueNameLen;
        CXX_EXCEPTION_IF(ErrorSyscall, ::RegQueryInfoKeyA(h,
            nullptr, nullptr, nullptr, &cSubKeys,
            &cbMaxSubKeyLen, nullptr, &cValues, &cMaxValueNameLen,
            nullptr, nullptr, nullptr));
        cbMaxSubKeyLen ++; 
        cMaxValueNameLen ++;
        if (mode_ & 1) {
            std::vector<char> KeyName(cbMaxSubKeyLen);
            for (DWORD i = 0; i < cSubKeys; ++i) {

                CXX_EXCEPTION_IF(ErrorSyscall, ::RegEnumKeyA(
                    h, i, KeyName.data(), cbMaxSubKeyLen));

                Value break_ = callback.Call(Context::Global(), Just(i), Just(KeyName.data()));
                if (break_.as_jsbool())
                    break;
            }
        }
        if (mode_ & 2) {
            std::vector<char> ValueName(cMaxValueNameLen);
            for (DWORD i = 0; i < cValues; ++i) {
                DWORD dwValueLen = cMaxValueNameLen;
                CXX_EXCEPTION_IF(ErrorSyscall, ::RegEnumValueA(
                    h, i, ValueName.data(), &dwValueLen, 0, 0, 0, 0));
                Value break_ = callback.Call(Context::Global(), Just(i), Just(ValueName.data()));
                if (break_.as_jsbool())
                    break;
            }
        }
        return nullptr;
    }

    using tcp_socket_t = JxxOf<tcp::socket>;
    using tcp_acceptor_t = JxxOf<tcp::acceptor>;
    using tcp_resolver_t = JxxOf<tcp::resolver>;

    _as_the<_Object> CreateAcceptor(
        _as_the<_String> addr,
        _as_the<_String> port,
        _as_the<_Object | _Optional> options)
    {
        JxxObjectPtr<tcp_acceptor_t> acceptor_ = new tcp_acceptor_t(current_io_context());
        if (!acceptor_) return JS_INVALID_REFERENCE;
        std::string remote_ = GetAs<std::string>(addr);
        std::string port_ = GetAs<std::string>(port);
        tcp::resolver resolver(current_io_context());
        tcp::endpoint endpoint = *resolver.resolve(remote_, port_).begin();
        acceptor_->open(endpoint.protocol());
        acceptor_->set_option(tcp::acceptor::reuse_address(true));
        acceptor_->bind(endpoint);
        return ExtObject::Create(acceptor_, nullptr);
    }

    //_as_the<_Object> Resolve(
    //    _as_the<_String> remote,
    //    _as_the<_String> port,
    //    _as_the<_Object | _Optional> options)
    //{
    //    JxxObjectPtr<tcp_acceptor_t> acceptor_ = new tcp_acceptor_t();
    //    if (!acceptor_) return JS_INVALID_REFERENCE;
    //    std::string remote_ = GetAs<std::string>(addr);
    //    std::string port_ = GetAs<std::string>(port);
    //    tcp::resolver resolver(current_io_context());
    //    tcp::endpoint endpoint = *resolver.resolve(remote_, port_).begin();
    //    acceptor_->open(endpoint.protocol());
    //    acceptor_->set_option(tcp::acceptor::reuse_address(true));
    //    acceptor_->bind(endpoint);
    //    return ExtObject::Create(acceptor_, nullptr);
    //}

    _as_the<_Object> Socket(
        _as_the<_Number> ip_ver,
        _as_the<_Object | _Optional> options)
    {
        int IP_VERSION = GetAs<int>(ip_ver);
        CXX_EXCEPTION_IF(JsErrorInvalidArgument, IP_VERSION != 4 && IP_VERSION != 6);
        JxxObjectPtr<tcp_socket_t> tcp_socket = new tcp_socket_t(current_io_context());
        CXX_EXCEPTION_IF(JsErrorOutOfMemory, !tcp_socket);
        std::error_code ec;
        tcp_socket->open(IP_VERSION == 4 ? tcp::v4() : tcp::v6(), ec);
        CXX_EXCEPTION_IF(ErrorAsioError, ec);
        return ExtObject::Create(tcp_socket, nullptr);
    }

    //_as_the<_Boolean> AsyncConnect(
    //    _as_the<_Object> socket,
    //    _as_the<_String> remote,
    //    _as_the<_String> port,
    //    _as_the<_Object | _Optional> options)
    //{
    //    Durable<ExtObject> jsSocket(socket);
    //    JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
    //    if (!cxxSocket) return Just(false);
    //    std::string remote_ = GetAs<std::string>(remote);
    //    std::string port_ = GetAs<std::string>(port);
    //    tcp::resolver resolver(cxxSocket->get_io_context());
    //    resolver.async_resolve(remote_, port_, [jsSocket, cxxSocket](std::error_code ec, auto it) {
    //        if (ec) {

    //        }
    //        else {
    //            cxxSocket->async_connect(it, [](std::error_code ec) {

    //                });
    //        });

    //    }

    //    return Just(true);
    //}

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
        if (!onConnection) return Just(false);
        auto cxxAcceptor = jsAcceptor.TryGetAs<tcp_acceptor_t>();
        if (!cxxAcceptor) return Just(false);
        cxxAcceptor->listen();
        if (!do_accept(jsAcceptor)) return Just(false);
        return Just(true);
    }

    _as_the<_Boolean> AsyncRead(_as_the<_Object> socket, _as_the<_BufferLike> buffer)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return Just(false);
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        if (!cxxSocket) return Just(false);
        Durable<value_ref_t> jsBuffer = buffer;
        content_t cxxBuffer = GetContent(jsBuffer.get());
        cxxSocket->async_read_some(asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer](std::error_code ec, size_t read)
            {
                Function callback = (value_ref_t)jsSocket->GetProperty("onData");
                if (!callback) return;
                callback.Call(
                    jsSocket.get(),
                    Just(ec.value()),
                    Just(read),
                    jsBuffer.get()
                );
            });
        return Just(true);
    };

    _as_the<_Boolean> AsyncReadEx(_as_the<_Object> socket, _as_the<_Number> bufferSize)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return Just(false);
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        if (!cxxSocket) return Just(false);
        Durable<ArrayBuffer> jsBuffer = ArrayBuffer::Alloc(GetAs<int>(bufferSize));
        if (!jsBuffer) return Just(false);
        content_t cxxBuffer = GetContent(jsBuffer.get());
        cxxSocket->async_read_some(
            asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer, bufferSize](std::error_code ec, size_t read)
            {
                Function callback = jsSocket->GetProperty("onData");
                if (!callback) return;
                callback.Call(
                    jsSocket.get(),
                    Just(ec.value()),
                    Just(read),
                    jsBuffer.get()
                );
                if (!ec)
                    AsyncReadEx(jsSocket.get(), bufferSize);
            });

        return Just(true);
    };

    _as_the<_Boolean> AsyncWrite(_as_the<_Object> socket, _as_the<_BufferLike> buffer)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return Just(false);
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
                    Just(ec.value()),
                    Just(written),
                    jsBuffer.get()
                );
            });
        return Just(true);
    }

}; // namespace js