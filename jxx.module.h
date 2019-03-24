#pragma once
// I have no idea to implement ES module,
// 
#include <unordered_map>
#include <shared_mutex>
#include <filesystem>
#include <deque>
#include <vector>
#include "js.object.h"
#include "jxx.api.h"
#include "jxx.class.h"

namespace fs = std::filesystem;

namespace jxx {

    class IModuleStroage {
    public:
        enum Kind {
            Directory = 0,
            Package = 1,        // *.jmp
        };
    public:
        virtual ~IModuleStroage() = 0;
        virtual const std::string_view token() = 0;
        virtual bool resolve(const js::StringView& specialer, fs::path& resolved, bool test) = 0;
    };

    using StringVector = std::vector<std::string>;

    using ModuleStroagePtr = std::shared_ptr<IModuleStroage>;

    class ModuleDirectory : public IModuleStroage {
    protected:
        std::string token_;
        fs::path    directory_;
    public:
        virtual const std::string_view token() {
            return std::string_view(token_);
        }
        virtual bool resolve(const js::StringView& specialer, fs::path& resolved) {
            fs::path full = directory_ / specialer;
            if (!fs::exists(full))
                return false;
            resolved = full.lexically_normal();
            return true;
        }
    protected:
        ModuleDirectory(std::string token, fs::path folder) {
            token_ = std::move(token);
            directory_ = std::move(folder);
        }
    public:
        static ModuleStroagePtr create(std::string specialer) {
            fs::path path(specialer);
            if (path.is_relative())
                path = fs::current_path() / path;
            return std::make_shared<ModuleDirectory>(std::move(specialer), std::move(path));
        }
    };

    using ModuleStorages = std::vector<ModuleStroagePtr>;

    class PathResolver {
    protected:
        ModuleStorages storages_;
    protected:
        bool exists(const std::string_view& specialer) {
            for (auto e : storages_) {
                if (e->token() == specialer)
                    return true;
            }
            return false;
        }
        auto end() {
            return storages_.end();
        }
    public:

        bool AddPath(std::string specialer) {
            if (exists(specialer)) return false;
            auto dir = ModuleDirectory::create(std::move(specialer));
            if (!dir) return false;
            storages_.emplace_back(std::move(dir));
            return true;
        }

        bool RemovePath(const std::string & specialer) {
            for (auto it = storages_.begin(); it != storages_.end(); ++it) {
                if ((*it)->token() == specialer) {
                    storages_.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool Resolve(const std::string & specialer, fs::path & resolved, bool test) {
            for (auto e : storages_) {
                if (e->resolve(specialer, resolved, test))
                    return true;
            }
            return false;
        }

        int ResolveEx(const StringVector & specialers, fs::path & resolved, bool test) {
            for (auto e : storages_) {
                for (int i = 0; i < specialers.size(); ++i) {
                    if (e->resolve(specialers[i], resolved, test))
                        return i;
                }
            }
            return -1;
        }
    };



    // running modules

    using ModuleExports = js::Durable<js::Object>;

    struct ActiveModule {
        enum State {
            Loading = 0,
            Loaded = 1,
        };
        enum Kind {
            kBuildinModule,
            kSourceModule,
            kNativeModule,
            kLinkToModule,
        };
        Kind            kind;
        State           state;
        js::String      name;
        js::String      path;
        ModuleExports   exports;
    };
};

class IJxxModuleDetail {
public:
    // specialer is a virutal name just likes "internals/fs"
    virtual JxxCharPtr Specialer() const = 0;
    virtual JxxErrorCode CreateModuleInstance(JsValueRef* exports) const = 0;
};

JXXAPI JxxErrorCode JxxLoadSourceModule(JxxCharPtr code, size_t length, JsValueRef* exports)
{
    auto runtime = JxxGetCurrentRuntime();
    js::Context context = JxxCreateContext(runtime);
    js::Context::Scope module_scope(context);
    js::Object(context.Global()).SetPrototype(runtime->module_proto_);
    JxxRunScript(buffer);
    *exports = context.Global()["exports"];
}

JXXAPI JxxErrorCode JxxEvalNativeModule(JxxCharPtr path, JsValueRef* exports)
{
    js::ArrayBuffer buffer = js::ArrayBuffer::CreateFrom(js::StringView(code, len));
    if (!buffer) return eoJsrt | 0;
    auto runtime = JxxGetCurrentRuntime();
    js::Context context = JxxCreateContext(runtime);
    js::Context::Scope module_scope(context);
    js::Object(context.Global()).SetPrototype(runtime->module_proto_);
    __LoadAndCallNativeModule(path);
    *exports = context.Global()["exports"];
}

class JxxBuildinSourceModule : public IJxxModuleDetail {
public:
    JxxCharPtr  specialer;
    JxxCharPtr  source;
    size_t      length;
    virtual JxxErrorCode CreateModuleInstance(JsValueRef* exports) const {
        return JxxLoadSourceModule(source, length, exports);
    }
    virtual JxxCharPtr Specialer() const {
        return specialer;
    }
};

using JxxNativeModuleFactory = JxxErrorCode(*)(JsValueRef* exports);

class JxxBuildinNativeModule : public IJxxModuleDetail {
public:
    JxxCharPtr  specialer;
    JxxNativeModuleFactory factory_;
    virtual JxxCharPtr Specialer() const {
        return specialer;
    }
    virtual JxxErrorCode CreateModuleInstance(JsValueRef* exports) const {
        return factory_(exports);
    }
};

class JxxAddonNativeModule : public IJxxModuleDetail {
public:
    JxxCharPtr  specialer;
    JxxCharPtr  path;
    virtual JxxCharPtr Specialer() const {
        return specialer;
    }
    virtual JxxErrorCode CreateModuleInstance(JsValueRef* exports) const {
        // exports is an ExternalObject, dlclose() will be called when exports is collecting.
        return JxxEvalNativeModule(path, exports);
    }
};

//  1. 内建模块注册表

using BuildinModuleFactory = const IJxxModuleDetail*;

class ModuleRegistry {
    using Registry = std::unordered_map<std::string_view, BuildinModuleFactory>;
protected:
    std::shared_mutex   lock_ = std::shared_mutex();
    Registry            registry_;
public:
    JxxErrorCode RegisterModule(BuildinModuleFactory detail, bool internal)
    {
        std::string_view key(detail->Specialer());
        std::unique_lock<std::shared_mutex> write_lock(lock_);
        auto it = registry_.find(key);
        if (it != registry_.end())
            return JxxErrorHasExisted;
        registry_[key] = detail;
        return JxxNoError;
    }
    BuildinModuleFactory QueryModule(JxxCharPtr specialer, bool internal) {
        std::string_view key(detail->Specialer());
        std::shared_lock<std::shared_mutex> read_lock(lock_);
        auto it = registry_.find(key);
        if (it == registry_.end())
            return nullptr;
        return it->second;
    }
};

using JxxModuleRegistry = JxxOf<ModuleRegistry>;

// Now, we need a running module manager

class JxxRuntimeModuleMgmt {
protected:

    using running_modules_t = std::unordered_map<fs::path, js::Durable<js::Value>>;

protected:

    JxxObjectPtr<ModuleRegistry> registry_;
    running_modules_t running_modules_;

public:

    js::Context CreateContextFor(JsValueRef uri) {
        js::Context ctx = JxxCreateContext();
        if (!ctx) return ctx;
        if (!global_proto) return ctx;
        js::Object global = ctx.Global();
        if (!global) return JS_INVALID_REFERENCE;
        if (!global.SetPrototype(global_proto))
            return JS_INVALID_REFERENCE;
        return ctx;
    }

    JxxErrorCode LoadSourceModule(js::StringView resolved, JsValueRef code, js::Value& exports) {
        auto it = running_modules_.find(resolved);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }
        js::Value uri = js::Just<js::StringView>(js::StringView(resolved.c_str()));
        if (!uri) return eoMemory | ecNotEnough;
        exports = js::Object::Create();
        running_modules_[resolved] = exports;
        exports = _EvalSourceModule(exports, uri, code);
        running_modules_[resolved] = exports;
        return ecNone;
    }

    JxxErrorCode LoadJsonModule(js::StringView resolved, JsValueRef code, js::Value& exports) {
        auto it = running_modules_.find(resolved);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }
        exports = JsonParse(code);
        running_modules_[resolved] = exports;
        return ecNone;
    }

    JxxErrorCode LoadNativeModule(js::StringView resolved, js::Value& exports) {
        auto it = running_modules_.find(resolved);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }
        exports = js::Object::Create();
        running_modules_[resolved] = exports;
        return ecNone;
    }

    JxxErrorCode LoadFileAsModule(js::StringView resolved, js::Value& exports) {
        auto it = running_modules_.find(resolved);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }
        exports = JxxReadFileContent(resolved.c_str(), 0);
        running_modules_[resolved] = exports;
        return ecNone;
    }

    JxxErrorCode LoadBuildinModule(js::StringView specialer, BuildinModuleFactory factory, js::Value& exports) {
        auto it = running_modules_.find(specialer);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }
        auto err = factory->CreateModuleInstance(exports);
        if (err) return err;
        running_modules_[specialer] = exports;
        return ecNone;
    }

    JxxErrorCode LoadModule(js::StringView specialer, js::Value& exports) {

        specialer.find_last_of('!');

        // 1. 在已加载模块中查找，找到则返回
        auto it = running_modules_.find(resolved);
        if (it != running_modules_.end()) {
            exports = it->second;
            return eoModule | ecHasExisted;
        }

        // 2. 看看是否是内置模块，是则加载内建模块
        BuildinModuleFactory bmf = registry_->QueryModule(specialer.data(), false);
        if (bmf)
            return LoadBuildinModule(specialer, bmf, exports);

        // 3. 搜索外部模块路径
        const std::vector<std::string> specialers = {
            /*0*/specialer,
            /*1*/specialer + ".js",
            /*2*/specialer + "/index.js",
            /*3*/specialer + "/package.json",
            /*4*/specialer + ".jsxm"
        };

        fs::path resolved;
        int index = search_path_.ResolveEx(specialers, resolved, false);
        if (index < 0) return eoPath | ecNotExists;
        if (fs::is_directory(resolved)) return eoFile | ecUnmatch;

        // 3.1. 处理package.json的情况
        if (resolved.filename() == "package.json") {
            // package.json
            js::Value json = JxxReadFileContent(resolved.c_str(), 0);
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

        // 4. 加载外部模块
        if (resolved != specialer) {
            auto it = running_modules_.find(resolved);
            if (it != running_modules_.end()) {
                exports = it->second;
                return eoModule | ecHasExisted;
            }
        }

        // 4.1. 加载源码模块
        auto ext = resolved.extension();
        if (ext == "js" || ext == "cjs") {
            auto code = JxxReadFileContent(resolved.c_str(), true);
            return LoadSourceModule(resolved, code, exports);
        }

        // 4.1. 加载json模块
        if (ext == "json") {
            auto data = JxxReadFileContent(resolved.c_str(), false);
            return LoadJsonModule(resolved, data, exports);
        }

        // 4.2. 加载AddOn模块
        if (ext == "jsxm")
            return LoadNativeModule(resolved, exports);

        // 4.3. 按二进制文件加载模块
        return LoadFileAsModule(resolved, exports);
    }
};