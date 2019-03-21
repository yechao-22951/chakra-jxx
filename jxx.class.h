#pragma once
#include "js.base.h"
#include "js.function.h"
#include "js.primitive.h"
#include "jxx.api.h"
#include "jxx.object.h"
#include <atomic>
#include <memory>
#include <vector>

using jxx_counter_t = std::atomic<uint32_t>;

struct JXX_VIRTUAL_POINT {};

template <typename This_, typename... Implements_>
class JxxClassTemplateNE : public Implements_... {
public:
    static inline JxxClassId __PARENTS__[] = {
        ((JxxClassId)JXX_DEFINITION_OF_<Implements_>)..., nullptr };
    static JxxExports __JS_METHODS__() { return {}; }
    static JxxExports __JS_FUNCTIONS__() { return {}; }
    static JxxParents __JS_PARENTS__() {
        return { sizeof...(Implements_), __PARENTS__ };
    };

protected:
    jxx_counter_t ref_count_ = 0;

public:
    virtual JxxRefCount AddRef() {
        auto rc = ++ref_count_;
        return rc;
    }
    virtual JxxRefCount Release() {
        auto rc = --ref_count_;
        if (!rc)
            delete this;
        return rc;
    }
    virtual void* QueryClass(JxxClassId clsid) {
        if (clsid == jxx_clsid_of_(This_))
            return this;
        void* ptrs_[] = { (Implements_::QueryClass(clsid))..., 0 };
        for (size_t i = 0; i < sizeof...(Implements_) + 1; ++i) {
            auto ptr = ptrs_[i];
            if (ptr)
                return ptr;
        }
        return nullptr;
    }
};

template <typename This_, typename... Implements_>
class JxxClassTemplate : public JxxClassTemplateNE<This_, Implements_...> {
public:
    static JxxExports __JS_METHODS__() {
        return JxxExports{ (JxxCount)__EXPORTED_METHODS__.size(),
                          __EXPORTED_METHODS__.empty()
                              ? nullptr
                              : __EXPORTED_METHODS__.data() };
    }
    static JxxExports __JS_FUNCTIONS__() {
        return JxxExports{ (JxxCount)__EXPORTED_FUNCTIONS__.size(),
                          __EXPORTED_FUNCTIONS__.empty()
                              ? nullptr
                              : __EXPORTED_FUNCTIONS__.data() };
    }

protected:
    static inline std::vector<JxxExport> __EXPORTED_METHODS__;
    static inline std::vector<JxxExport> __EXPORTED_FUNCTIONS__;
    static JXX_VIRTUAL_POINT ADD_EXPORT_METHOD(JxxNamePtr Name,
        JxxFunction JxxFunc) {
        __EXPORTED_METHODS__.push_back({ Name, JxxFunc });
        return JXX_VIRTUAL_POINT{};
    }
    static JXX_VIRTUAL_POINT ADD_EXPORT_FUNCTION(JxxNamePtr Name,
        JxxFunction JxxFunc) {
        __EXPORTED_FUNCTIONS__.push_back({ Name, JxxFunc });
        return JXX_VIRTUAL_POINT{};
    }

private:
    template <typename Function_, std::size_t... I>
    void ____call_magic_method(Function_ function, js::call_info_t& info,
        std::index_sequence<I...>) {
        js::param_t params_[] = { (js::param_t(info, I))..., {} };
        This_* this_ = (This_*)this;
        info.returnValue = (this_->*function)(info, params_[I]...);
    }

public:
    template <auto Method_, int Deny_ = DenyNew>
    static JsValueRef CHAKRA_CALLBACK _STUB_OF_MAGIC_METHOD_OF_(
        JsValueRef callee, bool isNew, JsValueRef * arguments,
        unsigned short argumentsCount, void* jxxClassId) {
        return js::xx::THROW_JS_EXCEPTION_([&]() {
            static const std::size_t N =
                ARG_COUNT_OF_<decltype(Method_)>::value;
            CXX_EXCEPTION_IF(JsErrorInvalidArgument, argumentsCount == 0);
            int mode = isNew ? js::DenyNew : js::DenyNormal;
            CXX_EXCEPTION_IF(js::ErrorCallModeIsDenied, mode & Deny_);
            js::ExtObject self(arguments[0]);
            CXX_EXCEPTION_IF(JsErrorInvalidArgument, !self);
            JxxObjectPtr<IJxxObject> JxxObject = self.GetJxxObject();
            CXX_EXCEPTION_IF(JsErrorInvalidArgument, !JxxObject);
            This_ * JxxThis = (This_*)JxxObject->QueryClass(jxxClassId);
            CXX_EXCEPTION_IF(JsErrorInvalidArgument, !JxxThis);
            js::call_info_t info = { callee,
                                    isNew,
                                    arguments[0],
                                    arguments + 1,
                                    (size_t)(argumentsCount - 1),
                                    jxxClassId };
            auto I___ = std::make_index_sequence<N>{};
            JxxThis->____call_magic_method(Method_, info, I___);
            return info.returnValue;
            });
    };
};

template <typename T>
class JxxOf : public JxxClassTemplate<JxxOf<T>, IJxxObject>, public T {
public:
    template < typename ...ARGS>
    JxxOf(ARGS&& ...args) : T(std::forward<ARGS>(args)...) {}
    T* get() { return this; };
};

template <typename T> void CHAKRA_CALLBACK CppDelete(void* ptr) {
    delete (T*)ptr;
}

#define JXX_EXPORT_METHOD(K, CXX_NAME)                                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_METHOD(                                                  \
            #CXX_NAME, &_STUB_OF_MAGIC_METHOD_OF_<&K::CXX_NAME, js::DenyNew>);

#define JXX_EXPORT_METHOD_RENAME(K, CXX_NAME, JS_NAME)                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##JS_NAME =                   \
        K::ADD_EXPORT_METHOD(                                                  \
            #JS_NAME, &_STUB_OF_MAGIC_METHOD_OF_<&K::CXX_NAME, js::DenyNew>);

#define JXX_EXPORT_FUNCTION(K, CXX_NAME)                                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            #CXX_NAME, &_STUB_OF_MAGIC_FUNC_OF_<&K::CXX_NAME, js::DenyNew>);

#define JXX_EXPORT_FUNCTION_RENAME(K, CXX_NAME, JS_NAME)                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            #JS_NAME, &_STUB_OF_MAGIC_FUNC_OF_<&K::CXX_NAME, js::DenyNew>);

JXXAPI int JxxMixinObject(JsValueRef object, JxxClassId clsid,
    int MixinOptions) {
    js::Object target(object);
    if (!target)
        return js::ErrorTypeMismatch;
    auto def = JxxQueryClass(clsid);
    for (size_t i = 0; i < def.Parents.Count; ++i) {
        auto err =
            JxxMixinObject(object, def.Parents.ClassIDs[i], MixinOptions);
        if (err < 0)
            return err;
    }
    if (MixinOptions & MIXIN_METHOD) {
        for (size_t i = 0; i < def.Methods.Count; ++i) {
            auto& item = def.Methods.Entries[i];
            JsValueRef name = js::Just(item.Name);
            if (!name)
                return JsErrorOutOfMemory;
            js::Function fn =
                js::Function::JsrtNative(item.Callee, name, clsid);
            if (!fn)
                return JsErrorOutOfMemory;
            target.SetProperty(js::PropertyId(item.Name), fn);
        }
    }
    if (MixinOptions & MIXIN_FUNCTION) {
        for (size_t i = 0; i < def.Functions.Count; ++i) {
            auto& item = def.Functions.Entries[i];
            JsValueRef name = js::Just(item.Name);
            if (!name)
                return JsErrorOutOfMemory;
            js::Function fn =
                js::Function::JsrtNative(item.Callee, name, clsid);
            if (!fn)
                return JsErrorOutOfMemory;
            target.SetProperty(js::PropertyId(item.Name), fn);
        }
    }
    return 0;
}

JXXAPI JxxClassDefinition JxxQueryClass(JxxClassId clsid) {
    using func_t = JxxClassDefinition(*)();
    return ((func_t)clsid)();
}
