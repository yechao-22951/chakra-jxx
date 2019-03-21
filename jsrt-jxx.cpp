// jsrt-js.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <asio.hpp>
#include <iostream>
#include <filesystem>
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include "js.value.h"
#include "jxx.class.h"
#include "jxx.runtime.h"
#include "jxx.asio.tcp.h"

using namespace js;


value_ref_t on_data(
    call_info_t& info,
    _as_the<_Number> ec,
    _as_the<_Number> bytes,
    _as_the<_BufferLike> buffer)
{
    int err = GetAs<int>(ec);
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
    Object client)
{
    if (!client) return nullptr;
    Function onData = Function::Magic<on_data>(nullptr, 0);
    client.SetProperty("onData", onData);
    AsyncReadEx(client, Just<int>(512));
    return nullptr;
}

class Console : public JxxClassTemplate<Console, IJxxObject> {
public:
    DEFINE_CLASS_NAME("org.mode.buildin.console");

public:
    static value_ref_t log(js::call_info_t&, more_list_ args) {
        return JS_INVALID_REFERENCE;
    }
};
namespace fs = std::filesystem;
#define startWith _Starts_with
bool resolve(const std::string& specialer, std::filesystem::path& out) {
    fs::path current = fs::current_path();
    fs::path target(specialer);
    if (target.is_absolute()) return false;
    /*
    if (specialer in BuildinModuleMap ) {
        
    }
    */
    out = (current / target).lexically_normal();
    if (fs::exists(out))
        return true;
    out = (current / "node_modules" / target).lexically_normal();
    if (fs::exists(out))
        return true;
    return false;
}

int main() {
    fs::path yes;
    resolve("x64", yes);
    if( fs::is_directory(yes) ) {
        fs::path indexjs = yes / "index.js";
        if( fs::exists(indexjs) ) {
            
        }
    }
    JxxRuntime runtime(JsRuntimeAttributeNone, nullptr);
    runtime.InitJsonTool();
    auto vv = runtime.JsonParse("{9}", 2);

    Context context = JxxCreateContext(&runtime);
    Context::Scope scope(context);

    value_ref_t code = JxxReadFileContent("test.js", 0);
    JxxRunScript(code, js::Just("fly"));

    auto k = VoidPtr(HKEY_CURRENT_USER);
    Value hkey = _RegCreateKey(k, Just("Console\\Git Bash"), nullptr);
    Value q = _RegQueryValue(hkey, Just("FontFamily"), Just(0));
    _RegEnumKey(hkey, Just(3), Function::Magic<Console::log>());

    Object server = CreateAcceptor(Just("0.0.0.0"), Just("8080"), nullptr);
    server["onConnection"] = Function::Magic<on_client>();
    Listen(server);
    runtime.io_context().run();
    int a = 1;
    //value_ref_t str1 = JxxGetString("hello");
    //value_ref_t str2 = JxxGetString("hello");
    //value_ref_t str3 = Just("hello");
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
    //Function f = Function::Magic<jello>(nullptr, 0);
    //f.Call(buffer);
    //std::cout << "Hello World!\n";
}
