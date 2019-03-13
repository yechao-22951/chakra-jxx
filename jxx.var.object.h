#pragma once

#include "jxx.var.value.h"
#include "jxx.var.primitive.h"

namespace jxx {

	struct contents_t {
		ChakraBytePtr	data = 0;
		uint32_t		size = 0;

		operator ChakraBytePtr* () {
			return &data;
		}
		operator uint32_t* () {
			return &size;
		}
	};

	using PropId = js_value_t;
	using symbol_t = js_value_t;

	PropId make_prop_id(const wchar_t* name) {
		JsValueRef out;
		auto err = JsGetPropertyIdFromName(name, &out);
		if (err) jxx_throw_error(ERR_MSG(JxxOutOfMemory));
		return out;
	}
	PropId make_prop_id(symbol_t sym) {
		JsValueRef out;
		auto err = JsGetPropertyIdFromSymbol(sym, &out);
		if (err) jxx_throw_error(ERR_MSG(JxxOutOfMemory));
		return out;
	}

	template <uint64_t AcceptableTypeMark_>
	class object_accessor_ : public js_value_t {
	public:
		enum { __required_type_mask__ = AcceptableTypeMark_ };
	public:
		err_t set_property(const wchar_t* name, js_value_t value) {}

		IJxxNativeObject* GetExtenalData() {
			void* data = nullptr;
			auto err = JsGetExternalData(get(), &data);
			if (err) return nullptr;
			return (IJxxNativeObject*)data;
		}

		bool SetExtenalData(IJxxNativeObject* ptr) {
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
			CXX_EXCEPTION_IF(JsGetContextOfObject(get(), &ctx));
			return ctx;
		}

		bool WhenCollect(void* data, JsObjectBeforeCollectCallback pcb) {
			return !JsSetObjectBeforeCollectCallback(get(), data, pcb);
		}

		class Property
		{
		protected:
			js_value_t	this_;
			PropId	prop_id_;

		public:
			Property() {}
			Property(js_value_t target, PropId prop) : this_(target), prop_id_(prop) {
			}
			void operator = (const js_value_t& val)
			{
				CXX_EXCEPTION_IF(JsSetProperty(this_, prop_id_, val, true));
			}
			operator js_value_t() const
			{
				JsValueRef out;
				CXX_EXCEPTION_IF(JsGetProperty(this_, prop_id_, out.address()));
				return js_value_t(out);
			}
		};

		Property operator[](const __js_proto__& prop_id)
		{
			Property X(*this, make_prop_id(L"__proto__"));
			return X;
		}

		Property operator[](PropId prop_id)
		{
			return Property(*this, prop_id);
		}

		template< typename T>
		Property operator[](T&& x)
		{
			return Property(*this, make_prop_id(x));
		}

		bool SetProperty(PropId pid, const js_value_t& v) {
			return JsSetProperty(*this, pid, v, false) == JsNoError;
		}
		js_value_t GetProperty(PropId pid) {
			JsValueRef out;
			CXX_EXCEPTION_IF(JsGetProperty(*this, pid, &out));
			return out;
		}
		js_value_t GetPrototype() {
			JsValueRef out;
			CXX_EXCEPTION_IF(JsGetPrototype(*this, out));
			return out;
		}
		void SetPrototype(js_value_t proto) {
			CXX_EXCEPTION_IF(JsSetPrototype(*this, proto));
		}
	};



	class array_accessor_ : public object_accessor_<_Array> {
	public:
		err_t set_item(const wchar_t* name, js_value_t value) {}
		js_value_t get_item(const wchar_t* name) {}
		err_t length(js_value_t value) {}
		js_value_t operator[](int index) {}
	};

	using Object = value_as_<object_accessor_<_Object>>;
	using AnyObject = value_as_<object_accessor_<_AnyObject>>;
	using Array = value_as_<array_accessor_>;

	template <JsTypedArrayType ElemType_> struct ArrayNativeType;

	template <> struct ArrayNativeType<JsArrayTypeFloat64> {
		using value_type = double;
	};
	template <> struct ArrayNativeType<JsArrayTypeFloat32> {
		using value_type = float;
	};
	template <> struct ArrayNativeType<JsArrayTypeInt8> {
		using value_type = int8_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeUint8> {
		using value_type = uint8_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeUint8Clamped> {
		using value_type = uint8_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeInt16> {
		using value_type = int16_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeUint16> {
		using value_type = uint16_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeInt32> {
		using value_type = int32_t;
	};
	template <> struct ArrayNativeType<JsArrayTypeUint32> {
		using value_type = uint32_t;
	};

	template <typename ElementType_> struct ElementTypeOfNativeType;

	template <> struct ElementTypeOfNativeType<int8_t> {
		enum { type_mask = 1 << JsArrayTypeInt8 };
	};
	template <> struct ElementTypeOfNativeType<uint8_t> {
		enum { type_mask = (1 << JsArrayTypeUint8) | (1 << JsArrayTypeUint8Clamped) };
	};
	template <> struct ElementTypeOfNativeType<int16_t> {
		enum { type_mask = 1 << JsArrayTypeInt16 };
	};
	template <> struct ElementTypeOfNativeType<uint16_t> {
		enum { type_mask = 1 << JsArrayTypeUint16 };
	};
	template <> struct ElementTypeOfNativeType<int32_t> {
		enum { type_mask = 1 << JsArrayTypeInt32 };
	};
	template <> struct ElementTypeOfNativeType<uint32_t> {
		enum { type_mask = 1 << JsArrayTypeUint32 };
	};
	template <> struct ElementTypeOfNativeType<float> {
		enum { type_mask = 1 << JsArrayTypeFloat32 };
	};
	template <> struct ElementTypeOfNativeType<double> {
		enum { type_mask = 1 << JsArrayTypeFloat64 };
	};

	// template < typename NativeElememtType_>
	// class _typed_array : public js_value_t {
	// public:
	//	enum { __required_type_mask__ =
	//ElementTypeOfNativeType<NativeElememtType_>::type_mask }; 	using value_type =
	//ArrayNativeType<ElememtType_>; public: 	operator [] () {

	//	}
	//};

	// using TypedArray = value_as_< _array >;

}; // namespace jxx


JXXAPI JsValueRef JxxCreateObject(JsValueRef prototype) {
	JsValueRef out = nullptr;
	auto err = JsCreateObject(&out);
	if (err) return nullptr;
	if (prototype) {
		err = JsSetPrototype(out, prototype);
		if (err) return nullptr;
	}
	return out;
}

JXXAPI JsValueRef JxxCreateExObject(JsValueRef prototype, IJxxNativeObject * native) {
	JsValueRef out = nullptr;
	auto err = JsCreateExternalObjectWithPrototype(native, JxxNativeObjectRelease, prototype, &out);
	if (err) return nullptr;
	if (native) native->AddRef();
	return out;
}