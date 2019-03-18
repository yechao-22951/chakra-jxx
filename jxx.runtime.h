#pragma once
#include "js.context.h"
#include "jxx.api.h"
#include "jxx.cc.h"
#include "jxx.class.h"
#include <asio.hpp>

class JxxRuntime : public JxxClassTemplateNE<JxxRuntime, IJxxObject> {
  protected:
    js::cache_t cache_;
    js::Runtime runtime_;
    asio::io_context loop_;

  public:
    JxxRuntime(JsRuntimeAttributes attr, JsThreadServiceCallback jts)
        : runtime_(attr, jts), loop_(1) {
        JsSetPromiseContinuationCallback(JxxRuntime::__PromiseContinuation,
                                         this);
    }
    ~JxxRuntime() { cache_.clear(); }

  public:
    js::cache_t &cache() { return cache_; };
    JsRuntimeHandle handle() { return runtime_; };
    void close() { JsDisposeRuntime(runtime_); }

  public:
    static void CHAKRA_CALLBACK
    BeforeCollectCallback(_In_opt_ void *callbackState) {
        ((JxxRuntime *)callbackState)->Release();
    }
    static void CALLBACK __PromiseContinuation(JsValueRef task, void *this_) {
        ((JxxRuntime *)this_)->PromiseContinuation(task);
    }
    void PromiseContinuation(JsValueRef task) {
        // post to event loop
        JsAddRef(task, nullptr);
        loop_.post([task]() {
            js::Context::Scope scope(js::Context::From(task));
            if (scope.HasEntered()) {
                JsValueRef global = js::Context::CurrentGlobal();
                JsCallFunction(task, &global, 1, nullptr);
            }
            JsRelease(task, nullptr);
        });
    }
};

JXXAPI JxxRuntime *JxxCreateRuntime(JsRuntimeAttributes attr,
                                    JsThreadServiceCallback jts) {
    JxxRuntime *out = new JxxRuntime(attr, jts);
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

JXXAPI JxxRuntime *JxxGetCurrentRuntime() {
    js::Context context = js::Context::Current();
    if (!context)
        return nullptr;
    JxxRuntime *rt = (JxxRuntime *)context.GetData();
    return rt;
}

JXXAPI JsContextRef JxxCreateContext(JxxRuntime *runtime) {
    return js::Context::Create(runtime->handle(), runtime);
}

JXXAPI JsValueRef JxxGetString(const char *ptr, size_t len) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime *rt = (JxxRuntime *)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    len = len ? len : strlen(ptr);
    return rt->cache().get_string(std::move(std::string(ptr, len)));
}

JXXAPI JsValueRef JxxQueryProto(const char *ptr) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime *rt = (JxxRuntime *)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    return rt->cache().get_proto(std::move(std::string(ptr)));
}

JXXAPI JsValueRef JxxRegisterProto(const char *ptr, JsValueRef proto) {
    js::Object prototype(proto);
    if (!prototype)
        return JS_INVALID_REFERENCE;
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime *rt = (JxxRuntime *)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    rt->cache().put_proto(std::string(ptr), prototype);
    return proto;
}

JXXAPI JsValueRef JxxGetSymbol(const char *ptr) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime *rt = (JxxRuntime *)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    return rt->cache().get_symbol(std::move(std::string(ptr)));
}