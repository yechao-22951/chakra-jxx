#pragma once
#include "js.base.h"

namespace js {

	class Context {
	protected:
		JsContextRef nake_ = nullptr;
	public:
		Context() {};

		Context(JsContextRef r) : nake_(r) {}

		JsContextRef get() { return nake_; }

		void set(JsContextRef v) { nake_ = v; }

		operator JsContextRef() const { return nake_; }

		operator bool() const { return nake_ != nullptr; }

		JsContextRef* addr() {
			return &nake_;
		}

		struct Scope {
			JsContextRef prev_ = JS_INVALID_REFERENCE;
			bool enteted_ = false;
			Scope(JsContextRef target) {
				JsContextRef prev = JS_INVALID_REFERENCE;
				auto err = JsGetCurrentContext(&prev_);
				if (err) return;
				err = JsSetCurrentContext(target);
				if (err) return;
				prev_ = prev;
				enteted_ = true;
			}
			~Scope() {
				if (enteted_) {
					JsSetCurrentContext(prev_);
				}
			}
		};

	public:

		static Context Current() {
			JsContextRef now_ = JS_INVALID_REFERENCE;
			auto err = JsGetCurrentContext(&now_);
			if (err) return JS_INVALID_REFERENCE;
			return now_;
		}

		static Context Create(JsRuntimeHandle handle, void* data) {
			JsContextRef out = JS_INVALID_REFERENCE;
			auto err = JsCreateContext(handle, &out);
			if (err) return out;
			if (!data) return out;
			err = JsSetContextData(out, data);
			if (err) return JS_INVALID_REFERENCE;
			return out;
		}

	};

}; // namespace js
