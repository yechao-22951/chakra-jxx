#pragma once
#include "jxx.api.h"
#include "jxx.var.h"
#include <string_view>
#include <string>

namespace jxx {

using Int = int;
using Real = double;
using StringView = std::wstring_view;
using String = std::wstring;
using Boolean = bool;
using String8 = std::string;
using String8View = std::string_view;

template <typename Ty_> Ty_ get_as(JsValueRef ref);

template <> Int get_as<Int>(JsValueRef ref) {
    int out = 0;
    auto err = JsNumberToInt(ref, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}
template <> Real get_as<Real>(JsValueRef ref) {
    double out = 0;
    auto err = JsNumberToDouble(ref, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> String get_as<String>(JsValueRef ref) {
    const wchar_t *str = nullptr;
    size_t len = 0;
    auto err = JsStringToPointer(ref, &str, &len);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return std::move(String(str, len));
}

template <> StringView get_as<StringView>(JsValueRef ref) {
    const wchar_t *str = nullptr;
    size_t len = 0;
    auto err = JsStringToPointer(ref, &str, &len);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return std::move(StringView(str, len));
}

template <> Boolean get_as<Boolean>(JsValueRef ref) {
    bool out = false;
    auto err = JsBooleanToBool(ref, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <typename Ty_> JsValueRef just_is_(Ty_ i);

template <> JsValueRef just_is_<Int>(Int i) {
    JsValueRef out;
    auto err = JsIntToNumber(i, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> JsValueRef just_is_<Real>(Real i) {
    JsValueRef out;
    auto err = JsDoubleToNumber(i, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> JsValueRef just_is_<const String &>(const String &s) {
    JsValueRef out;
    auto err = JsPointerToString(s.c_str(), s.size(), &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> JsValueRef just_is_<String>(String s) {
    JsValueRef out;
    auto err = JsPointerToString(s.c_str(), s.size(), &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> JsValueRef just_is_<const StringView &>(const StringView &sv) {
    JsValueRef out;
    auto err = JsPointerToString(sv.data(), sv.size(), &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <> JsValueRef just_is_<const String8 &>(const String8 &s) {
    String tmp(s.begin(), s.end());
    return just_is_(tmp);
}

template <> JsValueRef just_is_<const char *>(const char *ptr) {
    String tmp(ptr, ptr + strlen(ptr));
    return just_is_(tmp);
}
template <> JsValueRef just_is_<const wchar_t *>(const wchar_t *ptr) {
    JsValueRef out;
    auto err = JsPointerToString(ptr, wcslen(ptr), &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}
// template <>
// JsValueRef just_is_<const StringView&>(const StringView& sv) {
//	JsValueRef out;
//	auto err = JsPointerToString(sv.data(), sv.size(), &out);
//	if (err) jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
//	return out;
//}

template <> JsValueRef just_is_(Boolean bv) {
    JsValueRef out;
    auto err = JsBoolToBoolean(bv, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

template <Boolean Val_> JsValueRef just_is_() {
    JsValueRef out;
    auto err = JsBoolToBoolean(Val_, &out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

JsValueRef just_null() {
    JsValueRef out;
    auto err = JsGetNullValue(&out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}
JsValueRef just_undefined() {
    JsValueRef out;
    auto err = JsGetUndefinedValue(&out);
    if (err)
        jxx_throw_error(ERR_MSG(JxxErrorTypeMismatch));
    return out;
}

}; // namespace jxx
