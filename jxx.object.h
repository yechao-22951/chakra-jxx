#pragma once
#include "js.object.h"
#include "jxx.api.h"

namespace js {

	static void CHAKRA_CALLBACK JxxRelease(void* p)
	{
		IJxxObject* ptr = (IJxxObject*)p;
		if (ptr) ptr->Release();
	}
	class ext_object_accessor : public object_accessor_<_Object> {
	public:
		error_t AssignJxxObject(IJxxObject* xobj) {
			IJxxObject* old = nullptr;
			auto err = JsGetExternalData(get(), (void**)&old);
			if (err) return err;
			err = JsSetExternalData(get(), xobj);
			if (err) return err;
			if (xobj) xobj->AddRef();
			if (old) old->Release();
			return JsNoError;
		}
		IJxxObject* GetJxxObject() {
			IJxxObject* out = nullptr;
			auto err = JsGetExternalData(get(), (void**)& out);
			if (err) return nullptr;
			return out;
		}
	public:
		static value_ref_t Create(IJxxObject* xobj, JsValueRef proto) {
			JsValueRef out = JS_INVALID_REFERENCE;
			auto err = JsCreateExternalObjectWithPrototype(xobj, JxxRelease, proto, &out);
			if (err) return value_ref_t(JS_INVALID_REFERENCE);
			xobj->AddRef();
			return value_ref_t(out);
		}
	};

	using ExternalObject = base_value_<ext_object_accessor>;

};