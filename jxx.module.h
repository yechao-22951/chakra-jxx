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
        auto find(const std::string_view& specialer) {
            auto it = std::find(std::begin(storages_), std::end(storages_),
        }
        bool exists(const std::string_view & specialer) {
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
                for (int i = 0; i < specialers.size(); ++ i) {
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
        Kind        kind;
        State       state;
        js::String  name;
        js::String  path;
        Exports     exports;
    };
};

class IJxxModuleDetail {
public:
    // specialer is a virutal name just likes "internals/fs"
    virtual JxxCharPtr Specialer() const = 0;
    virtual JxxErrorCode CreateModuleInstance(JsValueRef* exports) const = 0;
};

JXXAPI JxxErrorCode JxxLoadSourceModule(JxxCharPtr code, size_t length, JsValueRef* exports);
JXXAPI JxxErrorCode JxxEvalNativeModule(JxxCharPtr path, JsValueRef* exports);

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

using JxxModuleFactory = const IJxxModuleDetail*;

class ModuleRegistry {
    using Registry = std::unordered_map<std::string_view, JxxModuleFactory>;
protected:
    std::shared_mutex   lock_ = std::shared_mutex();
    Registry            registry_;
public:
    JxxErrorCode RegisterModule(JxxModuleFactory detail, bool internal)
    {
        std::string_view key(detail->Specialer());
        std::unique_lock<std::shared_mutex> write_lock(lock_);
        auto it = registry_.find(key);
        if (it != registry_.end())
            return JxxErrorHasExisted;
        registry_[key] = detail;
        return JxxNoError;
    }
    JxxModuleFactory QueryModule(JxxCharPtr specialer, bool internal) {
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
    JxxObjectPtr<ModuleRegistry> registry_;
    jxx::ModuleStorages search_path_;
public:
    Require( const std::string_view & specialer ) {
        auto it = running_.find(location);
        if (it == running_.end()) {
            return it->second.exports;
        }
        auto info = registry_.FindModule(path);
        auto exports = info.CreateModuleInstanec();
        running_[path] = {
            info.kind
            info.name,
            info.path,
            exports
        };
        return exports;
    }
};