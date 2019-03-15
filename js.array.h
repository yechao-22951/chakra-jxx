#pragma once
#include "js.object.h"

namespace js {

	class IndexedProperty
	{
	protected:
		value_ref_t	this_;
		value_ref_t	index_;
	public:
		IndexedProperty() {}
		IndexedProperty(value_ref_t target, value_ref_t index) : this_(target), index_(index) {
		}
		void operator = (const value_ref_t& val)
		{
			error_if<>(JsSetIndexedProperty(this_, index_, val));
		}
		operator value_ref_t() const
		{
			value_ref_t out;
			JsGetIndexedProperty(this_, index_, out.addr());
			return out;
		}
	};

	class array_accessor_ : public object_accessor_<_Array> {
	public:
		uint32_t Length() {
			propid_t id = make_prop_id(L"length");
			if (!id) return 0;
			value_ref_t length = GetProperty(id);
			if (!length.is<_Number>())
				return 0;
			return get_as_<Int>(length);
		}
		value_ref_t GetItem(Int index) {
			auto i = just_is_<Int>(index);
			value_ref_t out;
			JsGetIndexedProperty(get(), i, out.addr());
			return out;
		}
		bool SetItem(Int index, value_ref_t value) {
			auto i = just_is_<Int>(index);
			auto err = JsSetIndexedProperty(get(), i, value);
			return err == JsNoError;
		}
		IndexedProperty operator [] (size_t index) {
			return IndexedProperty(*this, just_is_<Int>(index));
		}
	public:
		static value_ref_t Create(uint32_t len) {
			value_ref_t out;
			JsCreateArray(len, out.addr());
			return out;
		}
	};

	using Array = base_value_<array_accessor_>;

}; // namespace js

