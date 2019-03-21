#pragma once
#include "js.base.h"
#include "js.value.h"
#include <string>
#include <string_view>

namespace js {

    using String = std::string;
    using StringView = std::string_view;

    template <typename Ty_> Ty_ GetAs(value_ref_t ref);

    template <> Int GetAs<Int>(value_ref_t ref) {
        int out = 0;
        auto err = JsNumberToInt(ref, &out);
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }
    template <> Real GetAs<Real>(value_ref_t ref) {
        double out = 0;
        auto err = JsNumberToDouble(ref, &out);
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <> String GetAs<String>(value_ref_t ref) {
        String out;
        size_t len = 0;
        JSERR_TO_EXCEPTION(JsCopyString(ref, nullptr, 0, &len));
        out.resize(len);
        JSERR_TO_EXCEPTION(JsCopyString(ref, out.data(), len, &len));
        return std::move(out);
    }

    template <> Boolean GetAs<Boolean>(value_ref_t ref) {
        bool out = false;
        auto err = JsBooleanToBool(ref, &out);
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <typename Ty_> value_ref_t Just(Ty_ i);

    template <> value_ref_t Just<Int>(Int i) {
        value_ref_t out;
        auto err = JsIntToNumber(i, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <> value_ref_t Just<unsigned long>(unsigned long i) {
        value_ref_t out;
        auto err = JsIntToNumber(i, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <> value_ref_t Just<size_t>(size_t i) {
        value_ref_t out;
        auto err = JsIntToNumber((int)i, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <> value_ref_t Just<Real>(Real i) {
        value_ref_t out;
        auto err = JsDoubleToNumber(i, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <> value_ref_t Just<const String&>(const String& s) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(s.c_str(), s.size(), out.addr()));
        return out;
    }

    template <> value_ref_t Just<String>(String s) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(s.c_str(), s.size(), out.addr()));
        return out;
    }

    template <> value_ref_t Just<const CharPtr>(const CharPtr ptr) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(ptr, strlen(ptr), out.addr()));
        return out;
    }
    template <> value_ref_t Just<CharPtr>(CharPtr ptr) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(ptr, strlen(ptr), out.addr()));
        return out;
    }
    template <> value_ref_t Just<char*>(char* ptr) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(ptr, strlen(ptr), out.addr()));
        return out;
    }

    template <> value_ref_t Just<const StringView&>(const StringView& view) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(view.data(), view.size(), out.addr()));
        return out;
    }
    template <> value_ref_t Just<StringView>(StringView view) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(view.data(), view.size(), out.addr()));
        return out;
    }

    template <> value_ref_t Just(Boolean bv) {
        value_ref_t out;
        auto err = JsBoolToBoolean(bv, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <Boolean Val_> value_ref_t Just() {
        value_ref_t out;
        auto err = JsBoolToBoolean(Val_, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    value_ref_t VoidPtr(void* ptr) {
        value_ref_t out;
        auto err = JsDoubleToNumber((double)(intptr_t)ptr, out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

    template <typename P> 
    P VoidPtr(value_ref_t ref) {
        double out = 0;
        auto err = JsNumberToDouble(ref, &out);
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return (P)(intptr_t)out;
    }

    value_ref_t Null() {
        value_ref_t out;
        auto err = JsGetNullValue(out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }
    value_ref_t Undefined() {
        value_ref_t out;
        auto err = JsGetUndefinedValue(out.addr());
        CXX_EXCEPTION_IF(ErrorTypeMismatch, err);
        return out;
    }

}; // namespace js
