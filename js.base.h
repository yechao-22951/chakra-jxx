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

	using length_t = unsigned int;

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

	template <error_t code = ErrorJsrtError>
	void error_if(bool exp) {
		if (exp)
			throw exception_t(code);
	}
	template <error_t code = ErrorJsrtError>
	void error_if(JsErrorCode err) {
		if (err)
			throw exception_t(err);
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
		length_t size = 0;
		operator ChakraBytePtr* () { return &data; }
		operator length_t* () { return &size; }
		uint8_t & operator [] ( size_t i) {
			return data[i];
		}
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

	template < typename T>
	class Durable {
	protected:
		T ref_;
	public:
		Durable() {};
		Durable(T ref) : ref_(ref) {
			if(ref_) ref_->AddRef();
		}
		Durable(const Durable& r) {
			ref_ = r.ref_;
			if(ref_) ref_->AddRef();
		}
		Durable(Durable&& r) {
			ref_ = r.ref_;
			r.ref_ = nullptr;
		}
		~Durable() {
			if (ref_) ref_->Release();
		}
		operator T () {
			return ref_;
		}
		operator const T () const {
			return ref_;
		}
		T operator -> () {
			return ref_;
		}
		const T operator -> () const {
			return ref_;
		}

	};

	template <typename FN> struct ARG_COUNT_OF_;
	template <typename R, typename This_, typename... ARGS>
	struct ARG_COUNT_OF_<R(This_::*)(ARGS...)> {
		static const size_t value = sizeof...(ARGS);
	};
	template <typename R, typename... ARGS>
	struct ARG_COUNT_OF_<R(*)(ARGS...)> {
		static const size_t value = sizeof...(ARGS);
	};


}; // namespace js