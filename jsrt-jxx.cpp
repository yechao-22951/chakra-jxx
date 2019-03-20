// jsrt-js.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <asio.hpp>
#include <iostream>
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include "js.value.h"
#include "jxx.class.h"
#include "jxx.runtime.h"
#include "jxx.asio.h"

using namespace js;


value_ref_t on_data(
    call_info_t& info,
    _as_the<_Number> ec,
    _as_the<_Number> bytes,
    _as_the<_BufferLike> buffer)
{
    int err = get_as_<int>(ec);
    if (err) return JS_INVALID_REFERENCE;
    ArrayBuffer to_send = ArrayBuffer::CreateFrom(
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "Server: nginx/1.13.3\r\n"
        "Connection: keep-alive\r\n"
        "\r\n");
    AsyncWrite(info.self, to_send);
    return JS_INVALID_REFERENCE;
}
value_ref_t on_client(
    call_info_t& info,
    _as_the<_Object> client) 
{
    Object jsClient(client);
    Function onData = Function::FromMagic<on_data>(nullptr, 0);
    jsClient.SetProperty("onData", onData);
    AsyncReadEx(client, just_is_<int>(512));
    return nullptr;
}

class Console : public JxxClassTemplate<Console, IJxxObject> {
public:
    DEFINE_CLASS_NAME("org.mode.buildin.console");

public:
    value_ref_t log(js::call_info_t&, more_list_ args) { return JS_INVALID_REFERENCE; }

public:
    JXX_EXPORT_METHOD(Console, log);
};

int main() {
    JxxRuntime runtime(JsRuntimeAttributeNone, nullptr);
    Context context = JxxCreateContext(&runtime);
    Context::Scope scope(context);
    Object server = CreateAcceptor(just_is_("0.0.0.0"), just_is_("8080"), nullptr);
    server.SetProperty("onConnection", Function::FromMagic<on_client>(nullptr, 0) );
    Listen(server);
    runtime.io_context().run();
    int a=  1;
    //value_ref_t str1 = JxxGetString("hello");
    //value_ref_t str2 = JxxGetString("hello");
    //value_ref_t str3 = just_is_("hello");
    //value_ref_t sym1 = JxxGetSymbol("hello");

    //JxxMixinObject(js_jello, jxx_clsid_of_(Console), MIXIN_METHOD);
    //js_jello.Call(Undefined());
    //Function logf = Function(js_jello["log"]);
    //auto xobj = ExtObject::Create(new Console, JS_INVALID_REFERENCE);
    //logf.Call(xobj);
    //ArrayBuffer buffer = ArrayBuffer::Alloc(1000);
    //content_t content = buffer.GetContent();
    //content[0] = '2';
    //Durable<Object> hold(buffer);
    //Function f = Function::FromMagic<jello>(nullptr, 0);
    //f.Call(buffer);
    //std::cout << "Hello World!\n";
}
