#pragma once

#include "jxx.var.object.h"
#include "jxx.var.value.h"

using JxxFunctionCallInfo = js::call_info_t;

using JXX_STD_FUNCTION = void (*)(JxxFunctionCallInfo&);

#define WHEN_RUN_ [&]()

enum JxxFunctionCallMode {
	DenyNew = 1,
	DenyNormal = 2,
};

template <typename Return_> struct IF_EXCEPTION_RETURN {
	Return_ defaultReturnValue;
	IF_EXCEPTION_RETURN(Return_ ret) : defaultReturnValue(ret) {}
	template <typename FunctionLike_>
	Return_ operator<<(const FunctionLike_& function) const {
		try {
			return function();
		}
		catch (...) {
			return defaultReturnValue;
		}
	}
	template <typename FunctionLike_>
	Return_ operator = (const FunctionLike_& function) const {
		try {
			return function();
		}
		catch (...) {
			return defaultReturnValue;
		}
	}
};

namespace js {

	template <typename I> class _native {};

	using more_ = param_t;

	template <typename FN> struct ARG_COUNT_OF_;
	template <typename R, typename This_, typename... ARGS>
	struct ARG_COUNT_OF_<R(This_::*)(ARGS...)> {
		static const size_t value = sizeof...(ARGS);
	};

	template <typename R, typename... ARGS> struct ARG_COUNT_OF_<R(*)(ARGS...)> {
		static const size_t value = sizeof...(ARGS);
	};

	static const auto _CallbackOptional = _Function | _Nothing;

	namespace internal_ {
		template <typename Function_, std::size_t... I>
		void ____call_cxx_function(Function_ function, JxxFunctionCallInfo& info,
			std::index_sequence<I...>) {
			js::param_t params_[] = { (js::param_t(info, I))..., {} };
			info.returnValue = function(params_[I]...);
		}
	} // namespace internal_
}; // namespace js

template <auto Funtion_, int Deny_ = DenyNew>
static JsValueRef CHAKRA_CALLBACK
JXX_CXX_FUNCTION_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
	unsigned short argumentsCount, void* jxxClassId) {
	return IF_EXCEPTION_RETURN(JS_INVALID_REFERENCE) << WHEN_RUN_{
		static const std::size_t N = ARG_COUNT_OF_<decltype(Funtion_)>::value;
		CXX_EXCEPTION_IF(argumentsCount == 0);
		int mode = isNew ? DenyNormal : DenyNew;
		if (mode & Deny_)
			return JxxExceptionCreate();
		JxxFunctionCallInfo info = {callee,
									isNew,
									arguments[0],
									arguments + 1,
									(size_t)(argumentsCount - 1),
									jxxClassId};
		auto I___ = std::make_index_sequence<N>{};
		js::internal_::____call_cxx_function(Funtion_, info, I___);
		return info.returnValue;
	};
}

template <JXX_STD_FUNCTION Funtion_, int Deny_ = DenyNew>
static JsValueRef
JXX_STD_FUNCTION_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
	unsigned short argumentsCount, void* jxxClassId) {
	return IF_EXCEPTION_RETURN(JS_INVALID_REFERENCE) << WHEN_RUN_{
		CXX_EXCEPTION_IF(argumentsCount == 0);
		int mode = isNew ? DenyNormal : DenyNew;
		if (mode & Deny_)
			return JxxExceptionCreate();
		JxxFunctionCallInfo info = {
			callee,    isNew, arguments[0], arguments + 1, argumentsCount - 1,
			jxxClassId};
		Funtion_(info);
		return info.returnValue;
	};
}

namespace js {

	class function_accessor_ : public object_accessor_<_Function> {
	public:
		template <typename... ARGS>
		value_ref_t Call(bool isNew, _as_the<_Object | _Optional> this_, ARGS... args) {
			JsValueRef argv[] = { this_, args... };
			JsValueRef out = JS_INVALID_REFERENCE;
			auto err = JsCallFunction(get(), argv, sizeof...(args) + 1, &out);
			// FIXME: handle js exception
			return out;
			// bool js_excep = false;
			// if( JsHasException(&js_excep) )
			//	Jsec
		}
	};

	using Function = base_value_<function_accessor_>;

	static Function MakeFunction(JsNativeFunction fn, JsValueRef name,
		JxxClassId clsid) {
		JsValueRef out = nullptr;
		JsCreateNamedFunction(name, fn, clsid, &out);
		return Function(out);
	}

	static Function MakeConstructor(JsNativeFunction fn, JsValueRef name,
		_as_the<_Object | _Optional> prototype,
		JxxClassId clsid) {
		Function ret = MakeFunction(fn, name, clsid);
		if (!ret)
			return ret;
		if (!prototype)
			return ret;
		if (ret.SetProperty(make_prop_id(L"prototype"), prototype))
			return ret;
		return Function();
	}

}; // namespace js