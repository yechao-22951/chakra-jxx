#pragma once
#include "js.base.h"
#include "js.value.h"
#include "js.primitive.h"

namespace js {

	using propid_t = value_ref_t;
	using symbol_t = value_ref_t;

	propid_t PropertyId(const CharPtr name) {
		JsValueRef out;
		JsCreatePropertyId(name, strlen(name), &out);
		return out;
	}

	propid_t PropertyId(symbol_t sym) {
		JsValueRef out;
		JsGetPropertyIdFromSymbol(sym, &out);
		return out;
	}

	template <uint64_t AcceptableTypeMark_>
	class object_accessor_ : public value_ref_t {
	public:
		static const auto __required_type_mask__ = AcceptableTypeMark_;
	public:

		JsContextRef GetContext() {
			JsContextRef ctx = JS_INVALID_REFERENCE;
			JsGetContextOfObject(get(), &ctx);
			return ctx;
		}

		bool WhenCollect(void* data, JsObjectBeforeCollectCallback pcb) {
			return !JsSetObjectBeforeCollectCallback(get(), data, pcb);
		}

		class Property
		{
		protected:
			value_ref_t	this_;
			propid_t	prop_id_;

		public:
			Property() {}
			Property(value_ref_t target, propid_t prop) : this_(target), prop_id_(prop) {
			}
			void operator = (const value_ref_t& val)
			{
				EXCEPTION_IF<>(JsSetProperty(this_, prop_id_, val, true));
			}
			operator value_ref_t() const
			{
				value_ref_t out;
				JsGetProperty(this_, prop_id_, out.address());
				return out;
			}
		};

		Property operator[](const __prototype__& prop_id)
		{
			Property X(*this, PropertyId("__proto__"));
			return X;
		}

		Property operator[](propid_t prop_id)
		{
			return Property(*this, prop_id);
		}

		template< typename T>
		Property operator[](const T& x)
		{
			return Property(*this, PropertyId(x));
		}

		bool SetProperty(propid_t pid, const value_ref_t& v) {
			return JsSetProperty(*this, pid, v, false) == JsNoError;
		}

		value_ref_t GetProperty(propid_t pid) {
			value_ref_t out;
			JsGetProperty(*this, pid, out.address());
			return out;
		}

		value_ref_t GetPrototype() {
			value_ref_t out;
			JsGetPrototype(*this, out.address());
			return out;
		}

		bool SetPrototype(value_ref_t proto) {
			return JsSetPrototype(*this, proto) == JsNoError;
		}
	};

	using Object = base_value_<object_accessor_<_AnyObject>>;
	using ObjectOnly = base_value_<object_accessor_<_Object>>;

}; // namespace js

