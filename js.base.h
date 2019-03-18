#pragma once
#include <ChakraCore.h>
#include <algorithm>
#include <exception>
#include <stdint.h>

namespace js {

const int ErrorJsrtError = 0;
const int ErrorTypeMismatch = -1;
const int ErrorCallJxxIsDenied = -2;
const int ErrorInvalidArrayIndex = -3;
const int ErrorNotImplement = -4;

using Int = int;
using Real = double;
using Boolean = bool;
using CharPtr = const char *;
using error_t = int;
using length_t = unsigned int;
using refcnt_t = uint32_t;

class exception_t : public std::exception {
  protected:
    error_t code_ = ErrorJsrtError;

  public:
    exception_t() = default;
    exception_t(error_t code) : code_(code) {}
    error_t get() { return code_; }
};

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define ERR_MSG(x) x, __FILE__ ":" STRINGIFY(__LINE__) " : " #x

static inline void throw_if_not(error_t err, bool exp) {
    if (!exp)
        throw exception_t(err);
}

template <error_t code = ErrorJsrtError> void EXCEPTION_IF(bool exp) {
    if (exp)
        throw exception_t(code);
}
template <error_t code = ErrorJsrtError> void EXCEPTION_IF(JsErrorCode err) {
    if (err)
        throw exception_t(err);
};

#define CXX_EXCEPTION_IF(code, cond)                                           \
    {                                                                          \
        do {                                                                   \
            if (cond)                                                          \
                throw ::js::exception_t(code);                                 \
        } while (0);                                                           \
    }

#define JSERR_TO_EXCEPTION(err)                                                \
    do {                                                                       \
        if (err)                                                               \
            throw exception_t(err);                                            \
    } while (0);

struct call_info_t {
    JsValueRef callee;
    bool is_new;
    JsValueRef self;
    JsValueRef *args;
    size_t argc;
    void *cookie;
    JsValueRef returnValue;
};

class param_t {
  protected:
    JsValueRef *args = nullptr;
    size_t argc = 0;

  public:
    param_t() = default;
    param_t(const call_info_t &info, size_t i) {
        args = i < info.argc ? info.args + i : nullptr;
        argc = i < info.argc ? info.argc - i : 0;
    }
    size_t size() const { return argc; }
    JsValueRef operator[](size_t index) const {
        if (index >= argc)
            return nullptr;
        return args[index];
    }
};

using more_list_ = param_t;

struct content_t {
    ChakraBytePtr data = nullptr;
    length_t size = 0;
    operator ChakraBytePtr *() { return &data; }
    operator length_t *() { return &size; }
    uint8_t &operator[](size_t i) { return data[i]; }
};

template <typename Return_> struct IF_EXCEPTION_RETURN {
    Return_ defaultReturnValue;
    IF_EXCEPTION_RETURN(Return_ ret) : defaultReturnValue(ret) {}
    template <typename FunctionLike_>
    Return_ operator<<(const FunctionLike_ &function) const {
        try {
            return function();
        } catch (...) {
            return defaultReturnValue;
        }
    }
    template <typename FunctionLike_>
    Return_ operator=(const FunctionLike_ &function) const {
        try {
            return function();
        } catch (...) {
            return defaultReturnValue;
        }
    }
};

template <typename T> class Durable {
  protected:
    T ref_;

  public:
    Durable(){};
    Durable(T ref) : ref_(ref) {
        if (ref_)
            ref_->AddRef();
    }
    Durable(const Durable &r) {
        ref_ = r.ref_;
        if (ref_)
            ref_->AddRef();
    }
    Durable(Durable &&r) { std::exchange(ref_, r.ref_); }
    ~Durable() {
        if (ref_)
            ref_->Release();
    }
    Durable &operator=(const Durable &r) {
        reset();
        ref_ = r.ref_;
        if (ref_)
            ref_->AddRef();
        return *this;
    }
    Durable &operator=(Durable &&r) {
        reset();
        std::exchange(ref_, r.ref_);
        return *this;
    }
    void reset() {
        if (!ref_)
            return;
        ref_->Release();
        ref_ = T();
    }
    operator T() { return ref_; }
    operator const T() const { return ref_; }
    T operator->() { return ref_; }
    const T operator->() const { return ref_; }
};

////
//
//    Dyamic Class
//

// enum MixinOptions {
//    MixinMember,
//    MixinStatic,
//};

// class INativeClass : public dos::IObject {
// public:
//    static dos::IObject* __CLASS__() { return nullptr; };
// public:
//    virtual WideCharPtr UncName() = 0;
//    // mixin member ( create a property)
//    // mixin static ( create a constructor)
//    virtual long Mixin(JsValueRef object, int mixin_options) = 0;
//    // new cxx instance
//    virtual long New(JsValueRef* args, size_t argc) = 0;
//};

}; // namespace js

template <typename FN> struct ARG_COUNT_OF_;
template <typename R, typename This_, typename... ARGS>
struct ARG_COUNT_OF_<R (This_::*)(ARGS...)> {
    static const size_t value = sizeof...(ARGS);
};
template <typename R, typename... ARGS> struct ARG_COUNT_OF_<R (*)(ARGS...)> {
    static const size_t value = sizeof...(ARGS);
};
