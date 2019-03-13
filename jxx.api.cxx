#pragma once
#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#ifdef USE_JXX
#define JXXAPI extern
#else
#define JXXAPI __declspec(dllexport)
#endif

#define __jxx_clsdef_of(K) (K::__DEFINITION__())
#define __jxx_clsid_of_(K) (K::__DEFINITION__)

#define JXX_EXPORT(K, name)                                                    \
    static inline const __DECLARED_POINT__ __jxx__##name = K::JXX_ADD_EXPORT(  \
        #name, ARG_COUNT_OF_<decltype(&K::name)>::value,                       \
        (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<decltype(&K::name), &K::name>);

#define JXX_EXPORT_BY_NAME(K, JS_NAME, CXX_NAME)                               \
    static inline const __DECLARED_POINT__ __jxx__##name = K::JXX_ADD_EXPORT(  \
        #JS_NAME, ARG_COUNT_OF_<decltype(&K::CXX_NAME)>::value,                \
        (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<decltype(&K::CXX_NAME),             \
                                           &K::CXX_NAME>);

#define JXX_HIDDEN_ME enum { __VISIBILITY__ = false };

using JXX_NAME = const char *;
using JXX_COUNT = unsigned short;
using JXX_CALLEE = void (*)(void *, const void **, size_t);
using JXX_MIXIN = int (*)(void *);
using JXX_QUERY_SUPER = bool (*)(void *);
using JXX_UINT = unsigned int;
using JXX_BOOL = bool;

struct JXX_CLASS_DEFINION;
typedef JXX_CLASS_DEFINION (*JXX_CLASS_ID)();

struct JXX_EXPORT {
    JXX_NAME Name;
    JXX_CALLEE Callee;
    JXX_COUNT ArgumentCount;
};
struct JXX_EXPORTS {
    JXX_COUNT Count;
    const JXX_EXPORT *Entries;
};
struct JXX_CLASS_DEFINION {
    const void *ClassId;
    JXX_NAME Name;
    const JXX_CLASS_ID *ParentClasses;
    const JXX_BOOL *ParentVisibility;
    JXX_EXPORTS Exports;
    JXX_MIXIN Mixin;
};

template <typename CXX> JXXAPI JXX_CLASS_DEFINION JXX_DEFINITION_OF_() {
    JXX_CLASS_DEFINION out;
    out.Name = typeid(CXX).name();
    out.ClassId = &CXX::__DEFINITION__;
    out.ParentClasses = CXX::__PARENTS__;
    out.ParentVisibility = CXX::__PARENTS_VISABLE__;
    out.Exports.Count = (JXX_COUNT)CXX::__EXPORTS__.size();
    out.Exports.Entries = out.Exports.Count ? CXX::__EXPORTS__.data() : nullptr;
    out.Mixin = CXX::__MIXIN__;
    return out;
}

#include "jxx_var.h"

namespace jxx {

const int null = 1 << 0;
const int undefined = 1 << 1;
const int string = 1 << 2;
const int number = 1 << 3;
const int object = 1 << 4;
const int native = 1 << 5;
const int function = 1 << 6;
const int boolean = 1 << 7;
const int optional = 1 << 8;
const int buffer = 1 << 9;
const int nothing = null | undefined | optional;
const int _Primitive = null | undefined | string | number;
const int anything = -1;

struct __DECLARED_POINT__ {};

struct jxx_var {
    void *ref_;
};

struct js_paramters_t {
    const void **argv = 0;
    int argc = 0;
    js_paramters_t() = default;
    js_paramters_t(const void **args_, size_t argc_, int i) {
        argv = args_ + i;
        argc = argc_ - i;
    }
    const void *operator[](int index) {
        return index < argc ? argv[index] : nullptr;
    }
};

template <int T> struct _the : jxx_var {
    _the(const void *){};
    _the(const js_paramters_t &) {
        // get first and check types
    }
};

using jxx_more_ = js_paramters_t;

template <typename T> struct js_cxx_ : jxx_var {
    js_cxx_(){};
    js_cxx_(const js_paramters_t &args);
};

template <typename FN> struct ARG_COUNT_OF_;
template <typename R, typename CXX, typename... ARGS>
struct ARG_COUNT_OF_<R (CXX::*)(ARGS...)> {
    enum { value = sizeof...(ARGS) };
    // constexpr static size_t const value = sizeof...(Args);
};

template <class CXX, typename... PARENTS> class JXX_CLASS : public PARENTS... {
  public:
    enum { __VISIBILITY__ = true };

  protected:
    enum { __PARENTS_COUNT__ = sizeof...(PARENTS) };
    static inline JXX_CLASS_ID __PARENTS__[] = {PARENTS::__DEFINITION__...,
                                                nullptr};
    static inline JXX_BOOL __PARENTS_VISABLE__[] = {
        (PARENTS::__VISIBILITY__)..., false};
    static inline std::vector<JXX_EXPORT> __EXPORTS__;

  protected:
    static __DECLARED_POINT__ JXX_ADD_EXPORT(JXX_NAME Name, JXX_COUNT argc,
                                             JXX_CALLEE JxxFunc) {
        __EXPORTS__.push_back({Name, JxxFunc, argc});
        return __DECLARED_POINT__{};
    }

  public:
    static int __MIXIN__(void *obj) {
        for (size_t i = 0; i < __PARENTS_COUNT__; ++i) {
            if (!__PARENTS_VISABLE__[i])
                continue;
            JXX_CLASS_ID &clsid = __PARENTS__[i];
            JXX_CLASS_DEFINION def = clsid();
            auto r = def.Mixin(obj);
            if (r < 0)
                return r;
        }
        for (auto &exp : __EXPORTS__) {
        }
        return 0;
    }
    JXXAPI static JXX_CLASS_DEFINION __DEFINITION__() {
        JXX_CLASS_DEFINION out;
        out.Name = typeid(CXX).name();
        out.ClassId = &__DEFINITION__;
        out.ParentClasses = __PARENTS__;
        out.ParentVisibility = __PARENTS_VISABLE__;
        out.Exports.Count = (JXX_COUNT)__EXPORTS__.size();
        out.Exports.Entries = out.Exports.Count ? __EXPORTS__.data() : nullptr;
        out.Mixin = __MIXIN__;
        return out;
    }

  private:
    template <typename FUNC, std::size_t... I>
    jxx_var invoke_jxx_native_method____(FUNC fn, const void **args,
                                         size_t argc,
                                         std::index_sequence<I...>) {
        CXX *this_ = (CXX *)this;
        js_paramters_t jxx_args[] = {js_paramters_t(args, argc, I)..., {}};
        return (this_->*fn)(jxx_args[I]...);
    }
    template <typename FUNC, std::size_t N>
    void *invoke_jxx_native_method__(FUNC fn, const void **args, size_t argc) {
        jxx_var ret = invoke_jxx_native_method____(
            fn, args, argc, std::make_index_sequence<N>{});
        return ret.ref_;
    }

  protected:
    template <typename FN, FN fn>
    static void *JXX_NATIVE_METHOD_OF_(CXX *this_, const void **args,
                                       size_t argc) {
        if (argc < ARG_COUNT_OF_<FN>::value)
            return;
        return this_->invoke_jxx_native_method__<FN, ARG_COUNT_OF_<FN>::value>(
            fn, args, argc);
    }
    void *__jxx_query_class(JXX_CLASS_ID clsid) {
        if (clsid == __jxx_clsid_of_(CXX))
            return this;
        void *parents_[] = {(PARENTS::_query_class(clsid))..., 0};
        for (size_t i = 0; i < __PARENTS_COUNT__; ++i) {
            if (!__PARENTS_VISABLE__[i])
                continue;
            auto ptr = parents_[i];
            if (ptr)
                return ptr;
        }
        return nullptr;
    }
};

template <typename T> class hidden : public T {
  public:
    JXX_HIDDEN_ME;
    static int __MIXIN__(void *obj) { return 0; }
};

class JxxObject : public JXX_CLASS<JxxObject> {
  protected:
    std::atomic<long> ref_count__ = 0;

  public:
    virtual void *QueryClass(JXX_CLASS_ID clsid) {
        return (void *)__jxx_query_class(clsid);
    }
    virtual long AddRef() { return ++ref_count__; }
    virtual long Release() {
        auto r = --ref_count__;
        if (!r)
            delete this;
        return r;
    }
    virtual void OnCollect(){};
    virtual void Clear() {}
};

class JxxPackable : public JXX_CLASS<JxxPackable> {
  public:
    JXX_HIDDEN_ME;
    virtual _the<_Primitive> pack(_the<boolean> is_move) = 0;
    virtual jxx_cxx_<JxxPackable> unpack(_the<_Primitive> packed) = 0;
};

class JxxAwaitableIo : public JXX_CLASS<JxxAwaitableIo> {
  public:
    virtual _the<_AnyOrNothing>
    aw_read(_the<_AnyOrNothing | optional> size) = 0;
    virtual _the<_AnyOrNothing> aw_write(_the<_AnyOrNothing> data) = 0;
    virtual _the<_AnyOrNothing> aw_close() = 0;
    JXX_EXPORT(JxxAwaitableIo, aw_read);
    JXX_EXPORT(JxxAwaitableIo, aw_write);
};

class JxxIo : public JXX_CLASS<JxxIo> {
  public:
    virtual _the<_AnyOrNothing> input() = 0;
    virtual _the<_AnyOrNothing> output(_the<_AnyOrNothing> data) = 0;
    JXX_EXPORT(JxxIo, input);
    JXX_EXPORT(JxxIo, output);
};

class JxxPipeline : public JXX_CLASS<JxxPipeline, JxxIo> {
  public:
    virtual _the<object> aw_read(_the<number> size, jxx_more_) = 0;
    virtual _the<number> aw_write(_the<object | buffer | string> buffer,
                                  jxx_more_) = 0;
    virtual _the<number> aw_close(_the<buffer | string> buffer, jxx_more_) = 0;
    JXX_EXPORT(JxxAwaitableIo, aw_read);
    JXX_EXPORT(JxxAwaitableIo, aw_write);
};

}; // namespace jxx
