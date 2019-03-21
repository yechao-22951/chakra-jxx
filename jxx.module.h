#pragma once
// I have no idea to implement ES module,
// 
#include "js.object.h"
#include "jxx.api.h"
#include "jxx.class.h"
#include <unordered_map>
#include <shared_mutex>

using Exports = js::Durable<js::Object>;

struct JxxModule {
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

using RunningModules = std::unordered_map<std::string, JxxModule>;

using JxxNativeModuleLoader = value_ref_t(*)(const char* name, const char* path, void* callbackData);

using JxxSourceModuleLoader = value_ref_t(*)(void* callbackData);

using JxxModuleLoader = value_ref_t(*)(const char* name, const char* path, void* callbackData);

struct JxxModuleDesc
{
    JxxModule::Kind kind;
    js::String  name;
    js::String  path;
    void* callbackData;
    JxxModuleLoader loader;
};

class _ModuleRegistry {
protected:
    std::shared_mutex   lock_ = std::shared_mutex();
    ModuleRegistryMap   modules_;
public:
    int RegisterModule(
        const char* name,
        const char* path,
        JxxModuleLoader loader,
        void* data)
    {

    }
    FindModule(const std::string&) {

    }
};

using ModuleRegistry = JxxOf<_ModuleRegistry>;

class JxxModuleManager {
protected:
    JxxObjectPtr<ModuleRegistry> registry_;
    RunningModules               running_;
public:
    Require(const std::string& location) {
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