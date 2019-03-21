#pragma once
#include <asio.hpp>
#include <fstream>
#include "js.context.h"
#include "jxx.api.h"
#include "jxx.cc.h"
#include "jxx.class.h"

class JxxRuntime : public JxxClassTemplateNE<JxxRuntime, IJxxObject> {
protected:
    js::cache_t cache_;
    js::Runtime runtime_;
    asio::io_context loop_;
    js::Durable<js::Context> zone_0_;
    js::Durable<js::Function> json_parse_;
    js::Durable<js::Function> json_stringify;
public:
    JxxRuntime(JsRuntimeAttributes attr, JsThreadServiceCallback jts)
        : runtime_(attr, jts), loop_(1) {
        JsSetPromiseContinuationCallback(JxxRuntime::__PromiseContinuation,
            this);
        zone_0_ = js::Context::Create(runtime_, nullptr);
    }
    ~JxxRuntime() {
        cache_.clear();
    }
public:
    js::cache_t& cache() { return cache_; };
    JsRuntimeHandle handle() { return runtime_; };
    void close() { JsDisposeRuntime(runtime_); }
    asio::io_context& io_context() { return loop_; };

    JsValueRef JsonParse(const char* ptr, size_t len) {
        if (!json_parse_) return JS_INVALID_REFERENCE;
        if (!len) len = strlen(ptr);
        js::Context::Scope scope(zone_0_.get());
        js::value_ref_t text = js::Just<const js::StringView&>(js::StringView(ptr, len));
        if (!text) return JS_INVALID_REFERENCE;
        JsValueRef args[] = { js::Context::Global(), text };
        js::value_ref_t out;
        auto err = JsCallFunction(json_parse_.get(), args, 2, out.addr());
        if (!err) return out;
        if (err == JsErrorScriptException) {
            js::value_ref_t exception;
            JsGetAndClearException(exception.addr());
        }
        return JS_INVALID_REFERENCE;
    }
    JsErrorCode InitJsonTool() {
        js::Context::Scope scope(zone_0_.get());
        js::Object global = js::Context::Global();
        if (!global) return JsErrorInvalidArgument;
        js::Object json = global["JSON"];
        if (!json) return JsErrorInvalidArgument;
        json_parse_ = js::Function(json.GetProperty("parse"));
        json_stringify = js::Function(json.GetProperty("stringify"));
        return JsNoError;
    }
public:
    static void CHAKRA_CALLBACK
        BeforeCollectCallback(_In_opt_ void* callbackState) {
        ((JxxRuntime*)callbackState)->Release();
    }
    static void CALLBACK __PromiseContinuation(JsValueRef task, void* this_) {
        ((JxxRuntime*)this_)->PromiseContinuation(task);
    }
    void PromiseContinuation(JsValueRef task) {
        // post to event loop
        JsAddRef(task, nullptr);
        loop_.post([task]() {
            js::Context::Scope scope(js::Context::From(task));
            if (scope.HasEntered()) {
                JsValueRef global = js::Context::Global();
                JsCallFunction(task, &global, 1, nullptr);
            }
            JsRelease(task, nullptr);
            });
    }
};

JXXAPI JxxRuntime* JxxCreateRuntime(JsRuntimeAttributes attr,
    JsThreadServiceCallback jts) {
    JxxRuntime* out = new JxxRuntime(attr, jts);
    if (!out)
        return nullptr;
    auto err = JsSetRuntimeBeforeCollectCallback(
        out->handle(), out, &JxxRuntime::BeforeCollectCallback);
    if (err) {
        delete out;
        return nullptr;
    };
    out->AddRef();
    return out;
}

JXXAPI JxxRuntime* JxxGetCurrentRuntime() {
    js::Context context = js::Context::Current();
    if (!context)
        return nullptr;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    return rt;
}

JXXAPI JsContextRef JxxCreateContext(JxxRuntime* runtime) {
    return js::Context::Create(runtime->handle(), runtime);
}

JXXAPI JsValueRef JxxGetString(const char* ptr, size_t len) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    len = len ? len : strlen(ptr);
    return rt->cache().get_string(std::move(std::string(ptr, len)));
}

JXXAPI JsPropertyIdRef JxxGetPropertyId(JxxCharPtr ptr, size_t len)
{
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    len = len ? len : strlen(ptr);
    return rt->cache().get_propid(std::move(std::string(ptr, len)));
}

JXXAPI JsValueRef JxxQueryProto(const char* ptr) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    return rt->cache().get_proto(std::move(std::string(ptr)));
}

JXXAPI JsValueRef JxxRegisterProto(const char* ptr, JsValueRef proto) {
    js::Object prototype(proto);
    if (!prototype)
        return JS_INVALID_REFERENCE;
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    rt->cache().put_proto(std::string(ptr), prototype);
    return proto;
}

JXXAPI JsValueRef JxxGetSymbol(const char* ptr) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    return rt->cache().get_symbol(std::move(std::string(ptr)));
}

JXXAPI JsValueRef JxxRunScript(JxxCharPtr code, size_t len, JsValueRef url)
{
    if (!len) len = strlen(code);
    js::StringView view(code, len);
    js::value_ref_t js_code;
    auto err = (JsCreateString(view.data(), view.size(), js_code.addr()));
    if (err) return JS_INVALID_REFERENCE;
    js::value_ref_t result;
    err = JsRun(js_code, 0, url, JsParseScriptAttributeNone, result.addr());
    if (err) return JS_INVALID_REFERENCE;
    return result;
}
JXXAPI JsValueRef JxxRunScript(JsValueRef code, JsValueRef url)
{
    js::value_ref_t result;
    auto err = JsRun(code, 0, url, JsParseScriptAttributeNone, result.addr());
    if (err) return JS_INVALID_REFERENCE;
    return result;
}
JXXAPI JsValueRef JxxJsonParse(JxxCharPtr code, size_t len)
{
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
}

JXXAPI JsValueRef JxxReadFileContent(JxxCharPtr path, size_t len)
{
    try {
        std::ifstream in(path);
        std::stringstream data;
        data << in.rdbuf();
        std::string code(data.str());
        return js::Just<js::String>(std::move(code));
    }
    catch (...) {
        return JS_INVALID_REFERENCE;
    }
}
