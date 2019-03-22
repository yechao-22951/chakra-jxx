#pragma once
#include <asio.hpp>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include "js.context.h"
#include "jxx.api.h"
#include "jxx.cc.h"
#include "jxx.class.h"

namespace fs = std::filesystem;
class JxxRuntime : public JxxClassTemplateNE<JxxRuntime, IJxxObject> {
protected:
    struct named_path_t {
    };
    using path_array_t = std::vector<fs::path>;
protected:
    js::cache_t cache_;
    js::Runtime runtime_;
    asio::io_context loop_;
    path_array_t search_path_;

    // js utils
    js::Durable<js::Context> zone_0_;
    js::Durable<js::Function> json_parse_;
    js::Durable<js::Function> json_stringify;

public:
    JxxRuntime(JsRuntimeAttributes attr, JsThreadServiceCallback jts)
        : runtime_(attr, jts), loop_(1) {
        JsSetPromiseContinuationCallback(JxxRuntime::__PromiseContinuation,
            this);
        zone_0_ = js::Context::Create(runtime_, nullptr);
        AppendSearchPath(".");
        AppendSearchPath("node_modules");
        AppendSearchPath("mode_modules.jmp");
    }
    ~JxxRuntime() {
        cache_.clear();
    }
protected:
    bool _is_search_path_exists(const fs::path& path) {
        auto it = std::find(std::begin(search_path_), std::end(search_path_), path);
        return it != std::end(search_path_);
    }
public:

    js::cache_t& cache() { return cache_; };
    JsRuntimeHandle handle() { return runtime_; };
    void close() { JsDisposeRuntime(runtime_); }
    asio::io_context& io_context() { return loop_; };

    /** Perform an edit on the document associated with this text editor.
     *
     * The given callback-function is invoked with an [edit-builder](#TextEditorEdit) which must
     * be used to make edits. Note that the edit-builder is only valid while the
     * callback executes.
     *
     * @param path A function which can create edits using an [edit-builder](#JxxRuntime.AppendSearchPath).
     * @return If operation is successfully.
     */
    bool AppendSearchPath(fs::path path) {
        if (path.is_relative())
            path = fs::current_path() / path;
        if (_is_search_path_exists(path))
            return true;
        search_path_.emplace_back(std::move(path));
        return true;
    }

    /** Perform an edit on the document associated with this text editor.
     *
     * The given callback-function is invoked with an [edit-builder](#TextEditorEdit) which must
     * be used to make edits. Note that the edit-builder is only valid while the
     * callback executes.
     *
     * @param path A function which can create edits using an [edit-builder](#JxxRuntime.AppendSearchPath).
     * @return If operation is successfully.
     */
    bool RemoveSearchPath(const fs::path& path) {
        auto it = std::find(std::begin(search_path_), std::end(search_path_), path);
        if (it == std::end(search_path_))
            return false;
        search_path_.erase(it);
        return true;
    }

    /** Perform an edit on the document associated with this text editor.
     *
     * The given callback-function is invoked with an [edit-builder](#TextEditorEdit) which must
     * be used to make edits. Note that the edit-builder is only valid while the
     * callback executes.
     *
     * @param path A function which can create edits using an [edit-builder](#JxxRuntime.AppendSearchPath).
     * @return If operation is successfully.
     */
    bool ResolvePath(const fs::path & path, fs::path & out) {

    }

    /**
     * @brief Parse an string to JSON object.
     *
     * @param ptr A pointer to target string.
     * @param len The length of the string.
     * @return JsValueRef The parsed result.
     */
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
    /**
     * @brief Initillize JSON parser and stringifer.
     *
     * @return JsErrorCode The error code.
     */
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

    JsValueRef LoadModule(JsValueRef code) {
        auto rt = JxxGetCurrentRuntime();
        js::Context module_context = js::Context::Create(runtime_, nullptr);
        if (!module_context) return JS_INVALID_REFERENCE;
        js::Context::Scope module_scope(module_context);
        if (!module_scope.HasEntered()) return JS_INVALID_REFERENCE;
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

/**
 * @brief Query prototype object from current runtime by special name.
 *
 * @param name
 * @return JsValueRef
 */
JXXAPI JsValueRef JxxQueryProto(const char* name) {
    js::Context context = js::Context::Current();
    if (!context)
        return JS_INVALID_REFERENCE;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    if (!rt)
        return JS_INVALID_REFERENCE;
    return rt->cache().get_proto(std::move(std::string(name)));
}

/**
 * @brief
 *
 * @param ptr
 * @param proto
 * @return JsValueRef
 */
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
