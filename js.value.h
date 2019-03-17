#pragma once
#include "js.base.h"

namespace js {

	enum { JsOptional = 62, JsInvalidValue = 63 };

	// JsValueType
	const uint64_t __BIT = 1;
	const uint64_t _Null = __BIT << JsNull;
	const uint64_t _Undefined = __BIT << JsUndefined;
	const uint64_t _Boolean = __BIT << JsBoolean;
	const uint64_t _Number = __BIT << JsNumber;
	const uint64_t _String = __BIT << JsString;
	const uint64_t _Object = __BIT << JsObject;
	const uint64_t _Function = __BIT << JsFunction;
	const uint64_t _Array = __BIT << JsArray;
	const uint64_t _Buffer = __BIT << JsArrayBuffer;
	const uint64_t _DataView = __BIT << JsDataView;
	const uint64_t _Symbol = __BIT << JsSymbol;
	const uint64_t _TypedArray = __BIT << JsTypedArray;
	const uint64_t _Optional = __BIT << JsOptional;
	const uint64_t _AnyObject =
		_Object | _Function | _Array | _Buffer | _DataView | _TypedArray;
	const uint64_t _Nothing = _Null | _Undefined | _Optional;
	const uint64_t _Primitive = _Null | _Undefined | _String | _Number;
	const uint64_t _AnyOrNothing = ~(__BIT <<JsInvalidValue);

	struct __prototype__ {};
	static const __prototype__ __proto__;

	class value_ref_t {
	protected:
		JsValueRef nake_ = nullptr;
	public:
		value_ref_t() {};
		value_ref_t(JsValueRef r) : nake_(r) {}
		JsValueRef get() { return nake_; }
		void set(JsValueRef v) { nake_ = v; }

		size_t type_id() {
			if (!nake_)
				return JsOptional;
			JsValueType temp_;
			auto err = JsGetValueType(nake_, &temp_);
			if (err)
				return JsInvalidValue;
			return temp_;
		}

		uint64_t type_mask() {
			if (!nake_)
				return __BIT << JsOptional;
			JsValueType temp_;
			auto err = JsGetValueType(nake_, &temp_);
			if (err)
				return __BIT << JsInvalidValue;
			return __BIT << temp_;
		}

		template <uint64_t TypeId_> bool is() { return type_id() == TypeId_; }

		bool is(uint64_t id) { return type_id() == id; }

		template <uint64_t TypeMask_> bool is_one_of() {
			return (type_mask() & TypeMask_) != 0;
		}

		bool is_one_of(uint64_t mask) { return (type_mask() & mask) != 0; }

		operator JsValueRef() const { return nake_; }

		operator bool() const { return nake_ != nullptr; }

		bool is_property_id(JsPropertyIdType* type) {
			JsPropertyIdType type_;
			auto err = JsGetPropertyIdType(nake_, &type_);
			if (err) return false;
			if (type)* type = type_;
			return true;
		}

		JsValueRef* address() {
			return &nake_;
		}

		JsValueRef* addr() {
			return &nake_;
		}

		value_ref_t* operator -> () {
			return this;
		}
		uint32_t AddRef() {
			uint32_t rc = 0;
			auto err = JsAddRef(nake_, &rc);
			return rc;
		}
		uint32_t Release() {
			uint32_t rc = 0;
			JsRelease(nake_, &rc);
			return rc;
		}

	};


	template <uint64_t TypeMask_> class _as_the : public value_ref_t {
	public:
		_as_the(JsValueRef js_val) : value_ref_t(js_val) {
			throw_if_not(ErrorTypeMismatch, is_one_of<TypeMask_>());
		}

		_as_the(const value_ref_t& right) : value_ref_t(right) {
			throw_if_not(ErrorTypeMismatch, is_one_of<TypeMask_>());
		}

		_as_the(const _as_the& right) : value_ref_t(right) {
		}

		template <uint64_t TypeMask2_>
		_as_the(const _as_the& right)
			: _as_the(right.js_val_) {
			throw_if_not(ErrorTypeMismatch, is_one_of<TypeMask_>());
		}

		_as_the(const param_t& param) {
			JsValueRef right = param[0];
			set(right);
			throw_if_not(ErrorTypeMismatch, is_one_of<TypeMask_>());
		}

		operator value_ref_t() const { return *(value_ref_t*)this; }
	};


	// Accessor_ should inherit from value_ref_t
	template <typename Accessor_, int Optional_ = 1>
	class base_value_ : public Accessor_ {
	public:
		base_value_() = default;
		base_value_(value_ref_t value) {
			CXX_EXCEPTION_IF(ErrorTypeMismatch, !set(value));
		}
		template <class K, int O> base_value_(base_value_<K, O> value) {
			CXX_EXCEPTION_IF(ErrorTypeMismatch, !set(value));
		}
		template <uint64_t TypeMask_> base_value_(_as_the<TypeMask_> argv) {
			CXX_EXCEPTION_IF(ErrorTypeMismatch, !set(argv));
		}
		bool set(value_ref_t value) {
			if (!value.is_one_of(Accessor_::__required_type_mask__ |
				(Optional_ ? _Optional : 0)))
				return false;
			Accessor_::set(value.get());
			return true;
		}
		//bool set(base_value_ value) {
		//	return set((value_ref_t)value.get());
		//}
		//template <typename K, int O>
		//bool set(base_value_<K, O> value) {
		//	return set((value_ref_t)value);
		//}

		template <uint64_t TypeMask_>
		bool set(_as_the<TypeMask_> argv) {
			return set((value_ref_t)argv);
		}
		base_value_* operator -> () {
			return this;
		}
		uint32_t AddRef() {
			if (!Accessor_::get()) return 0;
			uint32_t rc = 0;
			JsAddRef(Accessor_::get(), &rc);
			return rc;
		}
		uint32_t Release() {
			if (!Accessor_::get()) return 0;
			uint32_t rc = 0;
			JsAddRef(Accessor_::get(), &rc);
			return rc;
		}
	};

	class value_accessor_ : public value_ref_t {
	public:
		static const auto __required_type_mask__ = _AnyOrNothing;
	};

	class symbol_accessor_ : public value_ref_t {
	public:
		static const auto __required_type_mask__ = _Symbol;
	};

	using Value = base_value_<value_accessor_>;
	using Symbol = base_value_<symbol_accessor_>;

}; // namespace js
