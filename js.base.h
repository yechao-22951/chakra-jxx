#pragma once
#include <ChakraCore.h>
#include <exception>
#include <stdint.h>

namespace js {

	const int ErrorJsrtError = 0;
	const int ErrorTypeMismatch = -1;
	const int ErrorCallModeIsDenied = -2;
	const int ErrorInvalidArrayIndex = -3;

	using error_t = int;

	class exception_t : public std::exception {
	protected:
		error_t code_ = ErrorJsrtError;

	public:
		exception_t() = default;
		exception_t(error_t code) : code_(code) {}
		error_t get() { return code_; }
	};

#define CXX_EXCEPTION_IF(x, c)                                                 \
    {                                                                          \
        do {                                                                   \
            if (x)                                                             \
                throw exception_t(c);                                          \
        } while (0);                                                           \
    }

	static inline void throw_if_not(error_t err, bool exp) {
		if (!exp)
			throw exception_t(err);
	}

	template <error_t code = ErrorJsrtError> struct error_if {
		error_if() = default;
		error_if(bool exp) {
			if (exp)
				throw exception_t(code);
		}
		error_if(JsErrorCode err) {
			if (err)
				throw exception_t(err);
		}
	};

	struct call_info_t {
		JsValueRef callee;
		bool is_new;
		JsValueRef self;
		JsValueRef* args;
		size_t argc;
		void* cookie;
		JsValueRef returnValue;
	};

	class param_t {
	protected:
		JsValueRef* args = nullptr;
		size_t argc = 0;

	public:
		param_t() = default;
		param_t(const call_info_t& info, size_t i) {
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
		uint32_t size = 0;
		operator ChakraBytePtr* () { return &data; }
		operator uint32_t* () { return &size; }
	};

	class IExternalData {
	public:
		virtual long AddRef() = 0;
		virtual long Release() = 0;
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
		Return_ operator=(const FunctionLike_& function) const {
			try {
				return function();
			}
			catch (...) {
				return defaultReturnValue;
			}
		}
	};

}; // namespace js