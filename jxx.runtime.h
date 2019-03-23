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
#include "jxx.module.h"

class JxxRuntime : public JxxClassTemplateNE<JxxRuntime, IJxxObject> {
protected:
    using running_modules_t = std::unordered_map<fs::path, js::Durable<js::Value>>;
protected:
    // shared
    JxxObjectPtr<ModuleRegistry> registry_;
protected:
    // this runtime
    js::ValueCache cache_;
    js::Runtime runtime_;
    asio::io_context loop_;
    jxx::PathResolver search_path_;
    // js utils
    js::Durable<js::Context> zone_0_;
    js::Durable<js::Function> json_parse_;
    js::Durable<js::Function> json_stringify;
    running_modules_t running_modules_;
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

    js::ValueCache& cache() { return cache_; };
    JsRuntimeHandle handle() { return runtime_; };
    void close() { JsDisposeRuntime(runtime_); }
    asio::io_context& io_context() { return loop_; };

    JxxErrorCode nodejs_like_require_(const std::string_view& specialer) {
        const std::vector<std::string> specialers = {
            /*0*/specialer,
            /*1*/specialer + ".js",
            /*2*/specialer + "/index.js",
            /*3*/specialer + "/package.json",
            /*4*/specialer + ".jsxm"
        };
        fs::path resolved;
        int index = search_path_.ResolveEx(specialers, resolved, false);
        if (index < 0) return eoPath | ecNoExists;
        if (fs::is_directory(resolved)) return eoFile | ecUnmatch;
        if (index == 3) {
            // package.json
            value_ref_t json = JxxReadFileContent(resolved.c_str(), 0);
            if (!json) return eoFile | ecIoRead;
            js::Object package = JxxJsonParse(json);
            if (!package) return eoFile | ecBadFormat;
            js::Value main = package["main"];
            if (!main) return eoPivotalData | ecNotExists;
            if (!main.is(JsString)) return eoPivotalData | ecUnmatch;
            fs::path path_of_main = resolved.parent_path() / GetAs<js::String>(main);
            if (!fs::exists(path_of_main)) return eoFile | ecNotExists;
            if (fs::is_directory(path_of_main)) return eoFile | ecUnmatch;
            resolved = path_of_main;
        }
        auto ext = resolved.extension();
        value_ref_t exports;
        if (ext == "js" || ext == "cjs") {
            auto data = JxxReadFileContent(resolved.c_str(), 0);
            auto content = js::GetContent(data);
            return _LoadSourceModule(content.get_ptr<char>(), content.size, exports.addr());
        }
        else if (ext == "json") {
            auto data = JxxReadFileContent(resolved.c_str(), 0);
            auto content = js::GetContent(data);
            exports = JxxJsonParse(content.get_ptr<char>(), content.size);
            if (!content) return JxxErrorJsrtError;
            running_modules_[resolved] = exports;
        }
        else if (ext == "jsxm") {
            return _LoadNativeModule(resolved.c_str(), exports.addr());
        }
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
    bool AppendSearchPath(fs::path path) {
        return search_path_.AddPath(path);
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
        return search_path_.RemovePath(path);
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
    bool ResolvePath(const fs::path& path, fs::path& out) {

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

using JxxRuntimeInitializeCallback = JxxErrorCode(*) (JxxRuntime*);

JXXAPI JxxRuntime* JxxCreateRuntime(JsRuntimeAttributes attr,
    JsThreadServiceCallback jts, JxxRuntimeInitializeCallback init = nullptr) {
    JxxObjectPtr<JxxRuntime> out = new JxxRuntime(attr, jts);
    if (!out)
        return nullptr;

    auto err = JsSetRuntimeBeforeCollectCallback(
        out->handle(), out, &JxxRuntime::BeforeCollectCallback);

    if (err)
        return nullptr;

    if (init) {
        auto e = init(out);
        if (e) return nullptr;
    }

    return out.detach();
}

JXXAPI JxxRuntime* JxxGetCurrentRuntime() {
    js::Context context = js::Context::Current();
    if (!context)
        return nullptr;
    JxxRuntime* rt = (JxxRuntime*)context.GetData();
    return rt;
}

JXXAPI JsContextRef JxxCreateContext(JxxRuntime* runtime) {
    if (!runtime) return JS_INVALID_REFERENCE;
    auto handle = runtime->handle();
    if (!handle) return JS_INVALID_REFERENCE;
    return js::Context::Create(handle, runtime);
}

JXXAPI JsValueRef JxxAllocString(const char* str, size_t len) {
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
    return rt->cache().get_string(str, len);
}

JXXAPI JsPropertyIdRef JxxAllocPropertyId(JxxCharPtr ptr, size_t len)
{
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
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
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
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
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
    js::Object prototype(proto);
    if (!prototype) return JS_INVALID_REFERENCE;
    rt->cache().put_proto(std::string(ptr), prototype);
    return proto;
}

JXXAPI JsValueRef JxxAllocSymbol(const char* ptr) {
    JxxRuntime* rt = JxxGetCurrentRuntime();
    if (!rt) return JS_INVALID_REFERENCE;
    return rt->cache().get_symbol(std::string(ptr));
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
        return ArrayBuffer::CreateFrom(code);
    }
    catch (...) {
        return JS_INVALID_REFERENCE;
    }
}
