#pragma once
#include "jxx.api.h"
#include <ChakraCore.h>
#include <stdint.h>

namespace jxx {

using err_t = JsErrorCode;

enum { JsOptional = 62, JsInvalidValue = 63 };

// JsValueType
const uint64_t __BIT = 1;
const uint64_t _Null = __BIT << JsNull;
const uint64_t _Boolean = __BIT << JsBoolean;
const uint64_t _Undefined = __BIT << JsUndefined;
const uint64_t _String = __BIT << JsString;
const uint64_t _Number = __BIT << JsNumber;
const uint64_t _Object = __BIT << JsObject;
const uint64_t _Function = __BIT << JsFunction;
const uint64_t _Array = __BIT << JsArray;
const uint64_t _Buffer = __BIT << JsArrayBuffer;
const uint64_t _DataView = __BIT << JsDataView;
const uint64_t _Symbol = __BIT << JsSymbol;
const uint64_t _TypedArray = __BIT << JsTypedArray;
const uint64_t _Optional = __BIT << JsOptional;
const uint64_t _AnyObject =
    _Object | _Function | _Array | _Buffer | _DataView | _TypedArray;
const uint64_t _Nothing = _Null | _Undefined | _Optional;
const uint64_t _Primitive = _Null | _Undefined | _String | _Number;
const uint64_t _AnyOrNothing = ~JsInvalidValue;

struct __js_proto__ {};
static const __js_proto__ __proto__;

class js_value_t {
  public:
    JsValueRef nake_ = nullptr;

  public:
    js_value_t(){};
    js_value_t(JsValueRef r) : nake_(r) {}
    JsValueRef get() { return nake_; }
    void set(JsValueRef v) { nake_ = v; }

    size_t type_id() {
        if (!nake_)
            return JsOptional;
        JsValueType temp_;
        auto err = JsGetValueType(nake_, &temp_);
        if (err)
            return JsInvalidValue;
        return temp_;
    }

    uint64_t type_mask() {
        if (!nake_)
            return __BIT << JsOptional;
        JsValueType temp_;
        auto err = JsGetValueType(nake_, &temp_);
        if (err)
            return __BIT << JsInvalidValue;
        return __BIT << temp_;
    }

    template <uint64_t TypeId_> bool is() { return type_id() == TypeId_; }

    bool is(uint64_t id) { return type_id() == id; }

    template <uint64_t TypeMask_> bool is_one_of() {
        return (type_mask() & TypeMask_) != 0;
    }

    bool is_one_of(uint64_t mask) { return (type_mask() & mask) != 0; }

    operator JsValueRef() const { return nake_; }
    operator bool() const { return nake_ != nullptr; }
};

struct call_info_t {
    JsValueRef callee;
    bool is_new;
    JsValueRef self;
    JsValueRef *args;
    size_t argc;
    void *cookie;
    JsValueRef returnValue;
};

class the_param_t {
  protected:
    JsValueRef *args = nullptr;
    size_t argc = 0;

  public:
    the_param_t() = default;
    the_param_t(const call_info_t &info, size_t i) {
        args = info.args + i;
        argc = info.argc - i;
    }
    size_t size() const { return argc; }
    JsValueRef operator[](size_t index) const {
        if (index >= argc)
            return nullptr;
        return args[index];
    }
};

template <uint64_t TypeMask_> class _the : public js_value_t {
  public:
    _the(JsValueRef js_val) : js_value_t(js_val) {
        if (!is_one_of<TypeMask_>())
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }

    _the(const js_value_t &right) : js_value_t(right) {
        if (!is_one_of<TypeMask_>())
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }

    _the(const _the &right) : js_value_t(right) {}

    template <uint64_t TypeMask2_>
    _the(const _the &right) : _the(right.js_val_) {
        if (!is_one_of<TypeMask_>())
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }

    _the(const the_param_t &param) {
        JsValueRef right = param[0];
        set(right);
        if (!is_one_of<TypeMask_>())
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }

    operator js_value_t() const { return *(js_value_t *)this; }
};

// Accessor_ should inherit from js_value_t
template <typename Accessor_, int Optional_ = 1>
class value_as_ : public Accessor_ {
  public:
    value_as_() = default;
    value_as_(js_value_t value) {
        if (!set(value))
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }
    value_as_(const value_as_ &value) {
        if (!set(value))
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }
    template <class K, int O> value_as_(value_as_<K, O> value) {
        if (!set(value))
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }
    template <uint64_t TypeMask_> value_as_(_the<TypeMask_> argv) {
        if (!set(argv))
            jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    }

    bool set(js_value_t value) {
        if (!value.is_one_of(Accessor_::__required_type_mask__ |
                             (Optional_ ? _Optional : 0)))
            return false;
        Accessor_::set(value.get());
        return true;
    }
    bool set(value_as_ value) {
        set(value.get());
        return true;
    }
    template <typename K, int O> bool set(value_as_<K, O> value) {
        return set((js_value_t)value);
    }

    template <uint64_t TypeMask_> bool set(_the<TypeMask_> argv) {
        return set((js_value_t)argv);
    }
};

class value_accessor_ : public js_value_t {
  public:
    enum { __required_type_mask__ = _AnyOrNothing };
};

using Value = value_as_<value_accessor_>;

}; // namespace jxx
