#pragma once

#include "js.object.h"

namespace js {

	using normal_func_t = void (*)(call_info_t&);

	enum call_mode_t {
		DenyNew = 1,
		DenyNormal = 2,
	};

	namespace xx {

		template <typename Function_>
		JsValueRef THROW_JS_EXCEPTION_(const Function_& func) {
			try {
				return func();
			}
			catch (const exception_t&) {
			}
			return JS_INVALID_REFERENCE;
		}

		template <normal_func_t Funtion_, int Deny_ = DenyNew>
		static JsValueRef
			_STUB_OF_NORMAL_FUNC_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
				unsigned short argumentsCount, void* externalData) {

			return THROW_JS_EXCEPTION_([&]() {
				CXX_EXCEPTION_IF(JsErrorInvalidArgument, argumentsCount == 0);
				int mode = isNew ? DenyNew : DenyNormal;
				EXCEPTION_IF<ErrorCallJxxIsDenied>(mode & Deny_);
				call_info_t info = { callee,
									isNew,
									arguments[0],
									arguments + 1,
									(size_t)(argumentsCount - 1),
									externalData };
				Funtion_(info);
				return info.returnValue;
				});
		};

		template <typename Function_, std::size_t... I>
		void ____call_magic_function(Function_ function, call_info_t & info,
			std::index_sequence<I...>) {
			param_t params_[] = { (param_t(info, I))..., {} };
			info.returnValue = function(params_[I]...);
		}

		template <auto Funtion_, int Deny_ = DenyNew>
		static JsValueRef CHAKRA_CALLBACK
			_STUB_OF_MAGIC_FUNC_OF_(JsValueRef callee, bool isNew, JsValueRef * arguments,
				unsigned short argumentsCount, void* externalData) {

			return THROW_JS_EXCEPTION_([&]() {
				static const std::size_t N = ARG_COUNT_OF_<decltype(Funtion_)>::value;
				CXX_EXCEPTION_IF(JsErrorInvalidArgument, argumentsCount == 0);
				int mode = isNew ? DenyNew : DenyNormal;
				EXCEPTION_IF<ErrorCallJxxIsDenied>(mode & Deny_);
				call_info_t info = { callee,
									isNew,
									arguments[0],
									arguments + 1,
									(size_t)(argumentsCount - 1),
									externalData };
				auto I___ = std::make_index_sequence<N>{};
				____call_magic_function(Funtion_, info, I___);
				return info.returnValue;
				});
		}
	};

	class function_accessor_ : public object_accessor_<_Function> {
	public:
		template <typename... ARGS>
		value_ref_t Call(_as_the<_AnyObject | _Nothing> this_, ARGS... args) {
			JsValueRef argv[] = { this_, args... };
			value_ref_t out;
			auto err = JsCallFunction(get(), argv, sizeof...(args) + 1, out.addr());
			return out;
		}
		template <typename... ARGS>
		value_ref_t Construct(_as_the<_AnyObject | _Optional> this_, ARGS... args) {
			JsValueRef argv[] = { this_, args... };
			value_ref_t out;
			auto err =
				JsConstructObject(get(), argv, sizeof...(args) + 1, out.addr());
			return out;
		}
	public:
		static value_ref_t FromNativeFunction(JsNativeFunction func, JsValueRef name, void* data)
		{
			value_ref_t out;
			JsCreateNamedFunction(name, func, data, out.addr());
			return out;
		}

		template < normal_func_t Function_ >
		static value_ref_t FromNormal(JsValueRef name, void* data)
		{
			value_ref_t out;
			JsCreateNamedFunction(name, xx::_STUB_OF_NORMAL_FUNC_OF_<Function_>, data, out.addr());
			return out;
		}

		template < auto Function_ >
		static value_ref_t FromMagic(JsValueRef name, void* data)
		{
			value_ref_t out;
			JsCreateNamedFunction(name, xx::_STUB_OF_MAGIC_FUNC_OF_<Function_>, data, out.addr());
			return out;
		}
	};

	using Function = base_value_<function_accessor_>;

	//class Promise {
	//	Object		promise_;
	//	Function	resolve_;
	//	Function	reject_;
	//public:
	//	Promise() {
	//		JsCreatePromise(promise_.addr(), resolve_.addr(), reject_.addr());
	//	}
	//	bool Resolve(Value value) {
	//		value_ref_t global = Context::CurrentGlobal();
	//		if( !global ) return false;
	//		resolve_.Call(global, value);
	//		return true;
	//	}
	//	Reject() {

	//	}
	//};



}; // namespace js