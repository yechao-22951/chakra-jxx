#pragma once
#include "jxx.api.h"
#include "jxx.var.function.h"
#include "jxx.var.primitive.h"
#include <memory>
#include <vector>

template <typename From_, typename To_> To_ *query_cast(From_ *from) {
    return from->QueryService(jxx_clsid_of(To_));
}

template <typename Native_> class no_ptr_t {
  protected:
    Native_ *nake_ = nullptr;

  public:
    Native_ *get() { return nake_; }
    no_ptr_t() = default;

    no_ptr_t(Native_ *ptr) { reset(ptr, true); }
    no_ptr_t(const no_ptr_t &r) { reset(r.get(), true); }
    no_ptr_t(no_ptr_t &&r) { nake_ = r.detach(); }
    no_ptr_t(IJxxNativeObject *p) { reset(p, true); }
    template <typename K> no_ptr_t(const no_ptr_t<K> &r) { reset(r); }
    template <typename K> no_ptr_t(no_ptr_t<K> &&r) { reset(r); }
    //////////////////////////////////////////////////
    no_ptr_t &attach(Native_ *ptr) { return reset(ptr, false); }
    Native_ *detach() { return std::exchange(nake_, nullptr); }
    no_ptr_t &reset() {
        auto old = std::exchange(nake_, nullptr);
        if (old)
            old->Release();
        return *this;
    }
    no_ptr_t &reset(Native_ *ptr, bool ref) {
        auto old = std::exchange(nake_, ptr);
        if (ptr && ref)
            ptr->AddRef();
        if (old)
            old->Release();
        return *this;
    }
    template <typename K> no_ptr_t &reset(K *ptr, bool ref) {
        Native_ *np = query_cast(ptr);
        return reset(np, ref);
    }
    template <typename K> no_ptr_t &reset(const no_ptr_t<K> &ptr) {
        Native_ *np = query_cast(ptr.get());
        return reset(np, true);
    }
    template <typename K> no_ptr_t &reset(no_ptr_t<K> &&ptr) {
        Native_ *np = query_cast(ptr.get());
        if (np)
            ptr.detach();
        return reset(np, false);
    }
    //////////////////////////////////////////////////
    Native_ *operator->() { return nake_; }
    const Native_ *operator->() const { return nake_; }
    //////////////////////////////////////////////////
    operator bool() const { return nake_ != nullptr; }
};

struct JXX_VIRTUAL_POINT {};

template <typename This_, typename... Implements_>
class JxxClassTemplateNE : public Implements_... {
  public:
    void *_query_service(JxxClassId clsid) {
        if (clsid == jxx_clsid_of_(This_))
            return this;
        void *ptrs_[] = {(Implements_::_query_service(clsid))..., 0};
        for (size_t i = 0; i < sizeof...(Implements_) + 1; ++i) {
            auto ptr = ptrs_[i];
            if (ptr)
                return ptr;
        }
        return nullptr;
    }

  protected:
    static inline JXX_CLASS_ID __PARENTS__[] = {
        ((JXX_CLASS_ID)JXX_DEFINITION_OF_<Implements_>)..., nullptr};

  public:
    static JXX_EXPORTS __JS_METHODS__() { return {}; }
    static JXX_EXPORTS __JS_FUNCTIONS__() { return {}; }
    static JXX_PARENTS __JS_PARENTS__() {
        return {__PARENTS__, sizeof...(Implements_)};
    };

  protected:
    long ref_count_ = 0;

  public:
    virtual long AddRef() { return ++ref_count_; }
    virtual long Release() {
        auto c = --ref_count_;
        if (!c)
            Free();
        return c;
    }
    virtual void *QueryService(JxxClassId clsid) {
        return _query_service(clsid);
    }
    virtual void Free() {}
};

template <typename This_, typename... Implements_>
class JxxClassTemplate : public JxxClassTemplateNE<This_, Implements_...> {
  public:
    static JXX_EXPORTS __JS_METHODS__() {
        return JXX_EXPORTS{(JXX_COUNT)__EXPORTED_METHODS__.size(),
                           __EXPORTED_METHODS__.empty()
                               ? nullptr
                               : __EXPORTED_METHODS__.data()};
    }
    static JXX_EXPORTS __JS_FUNCTIONS__() {
        return JXX_EXPORTS{(JXX_COUNT)__EXPORTED_FUNCTIONS__.size(),
                           __EXPORTED_FUNCTIONS__.empty()
                               ? nullptr
                               : __EXPORTED_FUNCTIONS__.data()};
    }

  protected:
    static inline std::vector<JXX_EXPORT> __EXPORTED_METHODS__;
    static inline std::vector<JXX_EXPORT> __EXPORTED_FUNCTIONS__;
    static JXX_VIRTUAL_POINT ADD_EXPORT_METHOD(JXX_NAME Name, JXX_COUNT argc,
                                               JXX_CALLEE JxxFunc) {
        __EXPORTED_METHODS__.push_back({Name, JxxFunc, argc});
        return JXX_VIRTUAL_POINT{};
    }
    static JXX_VIRTUAL_POINT ADD_EXPORT_FUNCTION(JXX_NAME Name, JXX_COUNT argc,
                                                 JXX_CALLEE JxxFunc) {
        __EXPORTED_FUNCTIONS__.push_back({Name, JxxFunc, argc});
        return JXX_VIRTUAL_POINT{};
    }

  private:
    ///////////////////////////
    // for class method
    template <typename FN, std::size_t... I>
    void ____call_jxx_cxx_method(FN fn, JxxFunctionCallInfo &info,
                                 std::index_sequence<I...>) {
        This_ *this_ = (This_ *)this;
        jxx::the_param_t params_[] = {(jxx::the_param_t(info, I))..., {}};
        info.returnValue = (this_->*fn)(params_[I]...);
    }

  public:
    template <auto Method_>
    static JsValueRef CHAKRA_CALLBACK
    JXX_NATIVE_METHOD_OF_(JsValueRef callee, bool isNew, JsValueRef *arguments,
                          unsigned short argumentsCount, void *jxxClassId) {
        static const std::size_t N = ARG_COUNT_OF_<decltype(Method_)>::value;
        CXX_EXCEPTION_IF(argumentsCount == 0);
        jxx::Object self(arguments[0]);
        IJxxNativeObject *NativeObject =
            (IJxxNativeObject *)self.GetExtenalData();
        CXX_EXCEPTION_IF(NativeObject == nullptr);
        This_ *NativeObjectThis = (This_ *)NativeObject->QueryService(
            jxxClassId); // will cast to classCoo
        CXX_EXCEPTION_IF(!NativeObjectThis);
        JxxFunctionCallInfo info = {callee,
                                    isNew,
                                    arguments[0],
                                    arguments + 1,
                                    (size_t)(argumentsCount - 1),
                                    jxxClassId};
        This_ *this_ = (This_ *)NativeObjectThis;
        auto I___ = std::make_index_sequence<N>{};
        NativeObjectThis->____call_jxx_cxx_method(Method_, info, I___);
        return info.returnValue;
    }

  public:
    template <typename... ARGS>
    static jxx::js_value_t NewInstance(ARGS... args) {
        no_ptr_t<This_> ret(new This_(std::forward<ARGS &&>(args)...));
        if (!ret)
            return JS_INVALID_REFERENCE;
        jxx::Object instance = JxxCreateExObject(nullptr, ret.get());
        if (!instance)
            return instance;
        JxxMixinObject(instance, jxx_clsid_of_(This_));
        return instance;
    }

    static JsValueRef MakePrototype() {
        jxx::Object instance = jxx::ObjectCreate();
        JsValueRef out = nullptr;
        auto err = JsCreateObject(&out);
        if (err)
            return nullptr;
        JXX_CLASS_DEFINION def = JXX_DEFINITION_OF_<This_>();
        for (size_t i = 0; i < def.Exports.Count; ++i) {
            std::string name_a = def.Exports.Entries[i].Name;
            std::wstring name_w = std::wstring(name_a.begin(), name_a.end());
            JsPropertyIdRef pid = nullptr;
            err = JsGetPropertyIdFromName(name_w.c_str(), &pid);
            if (err)
                continue;
        }
    }
};

template <typename CXX> JXXAPI JXX_CLASS_DEFINION JXX_DEFINITION_OF_() {
    static const JXX_CLASS_DEFINION out = {
        (JxxClassId)&JXX_DEFINITION_OF_<CXX>, typeid(CXX).name(),
        CXX::__JS_METHODS__(), CXX::__JS_FUNCTIONS__(), CXX::__JS_PARENTS__()};
    return out;
}

template <typename T>
class JxxOf : public JxxClassTemplate<JxxOf<T>>, public T {
  public:
    T *get() { return this; };
};

template <typename T> void CHAKRA_CALLBACK CppDelete(void *ptr) {
    delete (T *)ptr;
}

#define JXX_EXPORT_METHOD(K, CXX_NAME)                                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_METHOD(                                                  \
            L#CXX_NAME, ARG_COUNT_OF_<decltype(&K::CXX_NAME)>::value,          \
            (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_METHOD_RENAME(K, CXX_NAME, JS_NAME)                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##JS_NAME =                   \
        K::ADD_EXPORT_METHOD(                                                  \
            L#JS_NAME, ARG_COUNT_OF_<decltype(&K::CXX_NAME)>::value,           \
            (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_FUNCTION(K, CXX_NAME)                                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            L#CXX_NAME, ARG_COUNT_OF_<decltype(&K::CXX_NAME)>::value,          \
            (JXX_CALLEE)&JXX_CXX_FUNCTION_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_FUNCTION_RENAME(K, CXX_NAME, JS_NAME)                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            L#JS_NAME, ARG_COUNT_OF_<decltype(&K::CXX_NAME)>::value,           \
            (JXX_CALLEE)&JXX_CXX_FUNCTION_OF_ & K::CXX_NAME);

JXXAPI long JxxMixinObject(JsValueRef object, JXX_CLASS_ID clsid) {
    jxx::Object target(object);
    if (!target)
        return JxxErrorTypeMismatch;
    auto def = JxxQueryClassDefintion(clsid);
    for (size_t i = 0; i < def.Parents.Count; ++i) {
        auto lr = JxxMixinObject(object, def.Parents.ClassIDs[i]);
        if (lr < 0)
            return JxxErrorMixin;
    }
    for (size_t i = 0; i < def.Methods.Count; ++i) {
        auto &item = def.Methods.Entries[i];
        JsValueRef name = jxx::just_is_(item.Name);
        if (!name)
            return JxxOutOfMemory;
        jxx::Function fn = jxx::MakeFunction(item.Callee, name, clsid);
        target.SetProperty(jxx::make_prop_id(item.Name), fn);
    }
    return 0;
}