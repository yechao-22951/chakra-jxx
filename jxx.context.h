#pragma once
#include "jxx.api.h"
#include <ChakraCore.h>
#include <stdint.h>
#include <algorithm>
#include "jxx.var.object.h"

namespace js {
	class Context {
	protected:
		JsContextRef ref_ = JS_INVALID_REFERENCE;
	public:
		Context() {
		}
		Context(const Context& right) {
			ref_ = right.ref_;
		}
		Context(Context&& right) {
			std::swap(ref_, right.ref_);
		}
		Context& operator = (const Context& right) {
			ref_ = right.ref_;
			return *this;
		}
		Context& operator = (Context&& right) {
			std::swap(ref_, right.ref_);
			return *this;
		}

		operator JsContextRef () const {
			return ref_;
		}
		JsContextRef get() const {
			return ref_;
		}
		uint32_t AddRef() {
			if (!ref_) return 0;
			uint32_t rc = 0;
			JsAddRef(ref_, &rc);
			return rc;
		}
		uint32_t Release(Context ctx) {
			if (!ref_) return 0;
			uint32_t rc = 0;
			JsAddRef(ref_, &rc);
			return rc;
		}
		JsErrorCode SetData(void* p) {
			return JsSetContextData(ref_, p);
		}
		void * GetData() {
			void * p = 0;
			if( JsGetContextData(ref_, &p) ) 
				return nullptr;
			return p;
		}
		Object Global() {
			JsValueRef global = JS_INVALID_REFERENCE;
			auto err = JsGetGlobalObject(&global);
			if (err) return JS_INVALID_REFERENCE;
			return global;
		}
		void reset() {
			ref_ = JS_INVALID_REFERENCE;
		}
	protected:
		Context(JsContextRef ctx) : ref_(ctx) {
		}
	public:
		static Context New(JsRuntimeHandle runtime) {
			JsContextRef out = JS_INVALID_REFERENCE;
			auto err = JsCreateContext(runtime, &out);
			if (err) return JS_INVALID_REFERENCE;
			return out;
		}
		static Context Attach(JsContextRef context) {
			return Context(context);
		}
		static Context From(JsValueRef value) {
			JsContextRef out = JS_INVALID_REFERENCE;
			auto err = JsGetContextOfObject(value, &out);
			if (err) return JS_INVALID_REFERENCE;
			return out;
		}
	};

	class ContextScope {
	protected:
		JsContextRef prev_ = JS_INVALID_REFERENCE;
		bool enter_ = false;
	public:
		ContextScope(JsContextRef context) {
			JsGetCurrentContext(&prev_);
			enter_ = (JsSetCurrentContext(context) == JsNoError);
		}
		~ContextScope() {
			if (prev_)
				JsSetCurrentContext(&prev_);
		}
		bool IsEntered() {
			return enter_;
		}
	};

	template <typename T >
	uint32_t AddRef(T x);
	template <typename T >
	uint32_t Release(T x);

	template <>
	uint32_t AddRef<Context>(Context ctx) {
		uint32_t rc = 0;
		JsAddRef(ctx.get(), &rc);
	}
	template <>
	uint32_t Release<Context>(Context ctx) {
		uint32_t rc = 0;
		JsAddRef(ctx.get(), &rc);
	}
};