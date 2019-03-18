#pragma once
#include <asio.hpp>
#include "js.function.h"
#include "jxx.class.h"
#include "js.primitive.h"
#include "js.arraybuffer.h"
#include "js.context.h"

namespace js
{
	using namespace asio::ip;

	class IAsioReadWrite : public JxxClassTemplateNE<IAsioReadWrite, IJxxObject> {
	public:
		virtual int async_read(_as_the<_Buffer> buffer, _as_the<_Function> callback);
		virtual int async_write(_as_the<_Buffer> buffer, _as_the<_Function> callback);
	};

	template < class AsioHandle_>
	class AsioReadWrite : public {
	protected:
		AsioHandle_	handle_;

	public:

		AsioReadWrite(asio::io_context& io) : handle_(io) {
		}

		virtual int async_read(_as_the<_Buffer | _DataView> buffer, _as_the<_Function> callback) {
			content_t recv_buffer = js::GetContent(buffer);
			js::Durable<value_ref_t> buffer_ = buffer;
			js::Durable<js::Function> callback_ = callback;
			asio::async_read(handle_, asio::buffer(recv_buffer.data, recv_buffer.size),
				[buffer_, callback_](asio::error_code ec) {
					js::Context context = js::Context::From(callback_);
					js::Context::Scope scope(context);
					if (scope.HasEntered()) {
						callback_->Call(
							js::Context::CurrentGlobal(),
							js::just_is_(ec.value()),
							buffer_);
					}
				});
		}

		virtual int async_write(_as_the<_Buffer | _DataView> buffer, _as_the<_Function> callback) {
			content_t recv_buffer;
			std::string data;
			if (buffer.is<_String>()) {
				data = js::get_as_<js::String>(buffer);
				recv_buffer.data = data.data();
				recv_buffer.size = data.size();
			}
			else {
				recv_buffer = js::GetContent(buffer);
			}
			js::Durable<value_ref_t> buffer_ = buffer;
			js::Durable<js::Function> callback_ = callback;
			asio::async_write(handle_, asio::const_buffer(recv_buffer.data, recv_buffer.size),
				[buffer_, callback_](asio::error_code ec) {
					js::Context context = js::Context::From(callback_);
					js::Context::Scope scope(context);
					if (scope.HasEntered()) {
						callback_->Call(
							js::Context::CurrentGlobal(),
							js::just_is_(ec.value()),
							buffer_);
					}
				});
		}

		virtual int async_write_str(_as_the<_String> buffer, _as_the<_Function> callback, _as_the<_String|_Optional> encoding ) {
		}
	};


	class TcpSocket : public JxxClassTemplateNE<TcpSocket, IAsioReadWrite>{
	public:
		TcpSocket(asio::io_context& io) {

		}
	};

	class UdpSocket : public JxxClassTemplateNE<UdpSocket, IJxxObject> {
	public:
		udp::socket	socket_;
	public:
		UdpSocket(asio::io_context& io) : socket_(io) {
		}
	};

	value_ref_t bind(
		_as_the<_Object> sock,
		_as_the<_String | _Null | _Undefined> addr,
		_as_the<_Number> port)
	{
		std::string addr_s = addr ? js::get_as_<js::String>(addr) : host_name();
		address as_addr = make_address(addr_s);
		int port_i = get_as_<int>(port);
		ExternalObject extobj(sock);
		if (!extobj) return JS_INVALID_REFERENCE;
		JxxObjectPtr<IJxxObject> socket = extobj.GetJxxObject();
		if (!socket) return JS_INVALID_REFERENCE;
		JxxObjectPtr<TcpSocket> tcp_socket(socket);
		if (tcp_socket) {
			tcp::endpoint target(as_addr, port_i);
			tcp_socket->socket_.bind(target);
			return JS_INVALID_REFERENCE;
		}
		JxxObjectPtr<UdpSocket> udp_socket(socket);
		if (udp_socket) {
			udp::endpoint target(as_addr, port_i);
			udp_socket->socket_.bind(target);
			return JS_INVALID_REFERENCE;
		}
		return JS_INVALID_REFERENCE;
	}

	value_ref_t async_read(_as_the<_Object> sock, _as_the<_Buffer | _DataView | _TypedArray> data, _as_the<_Function> callback) {
		ExternalObject extobj(sock);
		if (!extobj) return JS_INVALID_REFERENCE;
		JxxObjectPtr<IJxxObject> socket = extobj.GetJxxObject();
		if (!socket) return JS_INVALID_REFERENCE;
		JxxObjectPtr<TcpSocket> tcp_socket(socket);
		if (tcp_socket) {
			tcp_socket->
		}
		JxxObjectPtr<UdpSocket> udp_socket(socket);
		if (udp_socket) {

		}
		return JS_INVALID_REFERENCE;
	}

	value_ref_t async_read(_as_the<_Object> sock, _as_the<_Buffer | _DataView | _TypedArray> data) {
		JxxObjectPtr<JxxRuntime> rt = JxxGetCurrentRuntime();
		if (!rt) return JS_INVALID_REFERENCE;
		ExternalObject extobj(sock);
		if (!extobj) return JS_INVALID_REFERENCE;
		JxxObjectPtr<IJxxObject> socket = extobj.GetJxxObject();
		if (!socket) return JS_INVALID_REFERENCE;
		JxxObjectPtr<TcpSocket> tcp_socket(socket);
		if (tcp_socket) {
			tcp_socket->
		}
		JxxObjectPtr<UdpSocket> udp_socket(socket);
		if (udp_socket) {

		}
		return JS_INVALID_REFERENCE;
	}


};