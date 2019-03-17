#pragma once
#include "js.base.h"
#include "js.value.h"
#include <string_view>
#include <string>

namespace js {

	using WideStringView = std::wstring_view;
	using WideString = std::wstring;
	using OneByteString = std::string;
	using OneByteStringView = std::string_view;

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

	template <> WideString get_as_<WideString>(value_ref_t ref) {
		const wchar_t* str = nullptr;
		size_t len = 0;
		auto err = JsStringToPointer(ref, &str, &len);
		EXCEPTION_IF<ErrorTypeMismatch>(err);
		return std::move(WideString(str, len));
	}

	template <> WideStringView get_as_<WideStringView>(value_ref_t ref) {
		const wchar_t* str = nullptr;
		size_t len = 0;
		auto err = JsStringToPointer(ref, &str, &len);
		EXCEPTION_IF<ErrorTypeMismatch>(err);
		return std::move(WideStringView(str, len));
	}

	template <> Boolean get_as_<Boolean>(value_ref_t ref) {
		bool out = false;
		auto err = JsBooleanToBool(ref, &out );
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

	template <> value_ref_t just_is_<const WideString&>(const WideString& s) {
		value_ref_t out;
		auto err = JsPointerToString(s.c_str(), s.size(), out.addr());
		EXCEPTION_IF<ErrorTypeMismatch>(err);
		return out;
	}

	template <> value_ref_t just_is_<WideString>(WideString s) {
		value_ref_t out;
		auto err = JsPointerToString(s.c_str(), s.size(), out.addr());
		EXCEPTION_IF<ErrorTypeMismatch>(err);
		return out;
	}

	template <> value_ref_t just_is_<const WideStringView&>(const WideStringView& sv) {
		value_ref_t out;
		auto err = JsPointerToString(sv.data(), sv.size(), out.addr());
		EXCEPTION_IF<ErrorTypeMismatch>(err);
		return out;
	}

	template <> value_ref_t just_is_<const OneByteString&>(const OneByteString& s) {
		WideString tmp(s.begin(), s.end());
		return just_is_(tmp);
	}

	template <> value_ref_t just_is_<OneByteCharPtr>(OneByteCharPtr ptr) {
		WideString tmp(ptr, ptr + strlen(ptr));
		return just_is_(tmp);
	}
	template <> value_ref_t just_is_<WideCharPtr>(WideCharPtr ptr) {
		value_ref_t out;
		auto err = JsPointerToString(ptr, wcslen(ptr), out.addr());
		EXCEPTION_IF<ErrorTypeMismatch>(err);
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
