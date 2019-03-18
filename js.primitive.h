#pragma once
#include "js.base.h"
#include "js.value.h"
#include <string_view>
#include <string>

namespace js {

    using String = std::string;
    using StringView = std::string_view;

    template <typename Ty_> Ty_ get_as_(value_ref_t ref);

    template <> Int get_as_<Int>(value_ref_t ref) {
        int out = 0;
        auto err = JsNumberToInt(ref, &out);
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }
    template <> Real get_as_<Real>(value_ref_t ref) {
        double out = 0;
        auto err = JsNumberToDouble(ref, &out);
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    template <> String get_as_<String>(value_ref_t ref) {
        String out;
        size_t len = 0;
        JSERR_TO_EXCEPTION(JsCopyString(ref, nullptr, 0, &len));
        out.resize(len);
        JSERR_TO_EXCEPTION(JsCopyString(ref, out.data(), len, &len));
        return std::move(out);
    }

    template <> Boolean get_as_<Boolean>(value_ref_t ref) {
        bool out = false;
        auto err = JsBooleanToBool(ref, &out);
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    template <typename Ty_> value_ref_t just_is_(Ty_ i);

    template <> value_ref_t just_is_<Int>(Int i) {
        value_ref_t out;
        auto err = JsIntToNumber(i, out.addr());
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    template <> value_ref_t just_is_<Real>(Real i) {
        value_ref_t out;
        auto err = JsDoubleToNumber(i, out.addr());
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    template <> value_ref_t just_is_<const String&>(const String& s) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(s.c_str(), s.size(), out.addr()));
        return out;
    }

    template <> value_ref_t just_is_<String>(String s) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(s.c_str(), s.size(), out.addr()));
        return out;
    }

    template <> value_ref_t just_is_<CharPtr>(CharPtr ptr) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(ptr, strlen(ptr), out.addr()));
        return out;
    }

    template <> value_ref_t just_is_<const StringView&>(const StringView& view) {
        value_ref_t out;
        JSERR_TO_EXCEPTION(JsCreateString(view.data(), view.size(), out.addr()));
        return out;
    }

    template <> value_ref_t just_is_(Boolean bv) {
        value_ref_t out;
        auto err = JsBoolToBoolean(bv, out.addr());
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    template <Boolean Val_> value_ref_t just_is_() {
        value_ref_t out;
        auto err = JsBoolToBoolean(Val_, out.addr());
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }

    value_ref_t Null() {
        value_ref_t out;
        auto err = JsGetNullValue(out.addr());
        EXCEPTION_IF<ErrorTypeMismatch>(err);
        return out;
    }
    value_ref_t Undefined() {
        value_ref_t out;
        auto err = JsGetUndefinedValue(out.addr());
        EXCEPTION_IF<>(err);
        return out;
    }

}; // namespace js
