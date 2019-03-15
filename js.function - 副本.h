#pragma once

#include "js.object.h"

namespace js {

	enum call_mode_t {
		DenyNew = 1,
		DenyNormal = 2,
	};

	using cpp_func_t = void (*)(call_info_t&);

	template <typename I> class _native {};

	using more_list_ = param_t;

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
		void ____call_cxx_function(Function_ function, call_info_t& info,
			std::index_sequence<I...>) {
			param_t params_[] = { (param_t(info, I))..., {} };
			info.returnValue = function(params_[I]...);
		}

		template <typename Function_>
		JsValueRef CXX_EXCEPTION_TO_JS(const Function_& func) {
			try {
				return func();
			}
			catch (const exception_t & e) {
				// JsCreateError()
			}
			catch (const std::exception & e) {
			}
			return JS_INVALID_REFERENCE;
		}

		template <auto Funtion_, int Deny_ = DenyNew>
		static JsValueRef CHAKRA_CALLBACK
			JXX_CXX_FUNCTION_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
				unsigned short argumentsCount, void* externalData) {

			return internal_::CXX_EXCEPTION_TO_JS([&]() {
				static const std::size_t N = ARG_COUNT_OF_<decltype(Funtion_)>::value;
				CXX_EXCEPTION_IF(argumentsCount == 0);
				int mode = isNew ? DenyNormal : DenyNew;
				error_if<ErrorCallModeIsDenied>(mode & Deny_);
				call_info_t info = { callee,
									isNew,
									arguments[0],
									arguments + 1,
									(size_t)(argumentsCount - 1),
									externalData };
				auto I___ = std::make_index_sequence<N>{};
				internal_::____call_cxx_function(Funtion_, info, I___);
				return info.returnValue;
				});
		}

		template <cpp_func_t Funtion_, int Deny_ = DenyNew>
		static JsValueRef
			JXX_STD_FUNCTION_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
				unsigned short argumentsCount, void* externalData) {

			return internal_::CXX_EXCEPTION_TO_JS([&]() {
				CXX_EXCEPTION_IF(argumentsCount == 0);
				int mode = isNew ? DenyNormal : DenyNew;
				error_if<ErrorCallModeIsDenied>(mode & Deny_);
				call_info_t info = { callee,
									isNew,
									arguments[0],
									arguments + 1,
									(size_t)(argumentsCount - 1),
									externalData };
				Funtion_(info);
				return info.returnValue;
				};
		}

	} // namespace internal_

	class function_accessor_ : public object_accessor_<_Function> {
	public:
		template <typename... ARGS>
		value_ref_t Call(_as_the<_Object | _Optional> this_, ARGS... args) {
			JsValueRef argv[] = { this_, args... };
			value_ref_t out = JS_INVALID_REFERENCE;
			auto err = JsCallFunction(get(), argv, sizeof...(args) + 1, out.addr());
			// FIXME: handle js exception
			return out;
			// bool js_excep = false;
			// if( JsHasException(&js_excep) )
			//	Jsec
		}
		template <typename... ARGS>
		value_ref_t Construct(_as_the<_Object | _Optional> this_, ARGS... args) {
			JsValueRef argv[] = { this_, args... };
			value_ref_t out = JS_INVALID_REFERENCE;
			auto err =
				JsConstructObject(get(), argv, sizeof...(args) + 1, out.addr());
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