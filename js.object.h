#pragma once
#include "js.base.h"
#include "js.value.h"
#include "js.primitive.h"

namespace js {

	using propid_t = value_ref_t;
	using symbol_t = value_ref_t;

	propid_t make_prop_id(const WideCharPtr name) {
		JsValueRef out;
		JsGetPropertyIdFromName(name, &out);
		return out;
	}
	propid_t make_prop_id(symbol_t sym) {
		JsValueRef out;
		JsGetPropertyIdFromSymbol(sym, &out);
		return out;
	}

	template <uint64_t AcceptableTypeMark_>
	class object_accessor_ : public value_ref_t {
	public:
		enum { __required_type_mask__ = AcceptableTypeMark_ };
	public:
		error_t set_property(const wchar_t* name, value_ref_t value) {}

		IExternalData* GetExtenalData() {
			void* data = nullptr;
			auto err = JsGetExternalData(get(), &data);
			if (err) return nullptr;
			return (IExternalData*)data;
		}

		bool SetExtenalData(IExternalData* ptr) {
			auto old = GetExtenalData();
			if (old) old->Release();
			if (!ptr) return true;
			auto err = JsSetExternalData(get(), ptr);
			if (err) return false;
			ptr->AddRef();
			return true;
		}

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
				error_if<>(JsSetProperty(this_, prop_id_, val, true));
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
			Property X(*this, make_prop_id(L"__proto__"));
			return X;
		}

		Property operator[](propid_t prop_id)
		{
			return Property(*this, prop_id);
		}

		template< typename T>
		Property operator[](const T& x)
		{
			return Property(*this, make_prop_id(x));
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

	using Object = base_value_<object_accessor_<_Object>>;
	using AnyObject = base_value_<object_accessor_<_AnyObject>>;

}; // namespace js

