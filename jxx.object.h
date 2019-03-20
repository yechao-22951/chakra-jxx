#pragma once
#include "js.object.h"
#include "jxx.api.h"
#include "jxx.class.h"

namespace js {

    template <typename To_>
    To_* query_cast(IJxxObject* from) {
        return (To_*)from->QueryClass(jxx_clsid_of_(To_));
    }

    static void CHAKRA_CALLBACK JxxRelease(void* p) {
        IJxxObject* ptr = (IJxxObject*)p;
        if (ptr)
            ptr->Release();
    }
    class ext_object_accessor : public object_accessor_<_Object> {
    public:
        error_t AssignJxxObject(IJxxObject* xobj) {
            IJxxObject* old = nullptr;
            auto err = JsGetExternalData(get(), (void**)& old);
            if (err)
                return err;
            err = JsSetExternalData(get(), xobj);
            if (err)
                return err;
            if (xobj)
                xobj->AddRef();
            if (old)
                old->Release();
            return JsNoError;
        }
        IJxxObject* GetJxxObject() {
            IJxxObject* out = nullptr;
            auto err = JsGetExternalData(get(), (void**)& out);
            if (err)
                return nullptr;
            return out;
        }
        template < class JxxClass_>
        JxxObjectPtr<JxxClass_> TryGetAs() {
            IJxxObject* out = GetJxxObject();
            if( !out ) return nullptr;
            return query_cast<JxxClass_>(out);
        };

    public:
        static value_ref_t Create(IJxxObject* xobj, JsValueRef proto) {
            JsValueRef out = JS_INVALID_REFERENCE;
            auto err =
                JsCreateExternalObjectWithPrototype(xobj, JxxRelease, proto, &out);
            if (err)
                return value_ref_t(JS_INVALID_REFERENCE);
            xobj->AddRef();
            return value_ref_t(out);
        }

        template < class JxxClass_>
        struct As {
            template <typename ...ARGS>
            static value_ref_t New(JsValueRef proto, ARGS&& ... args) {
                JxxObjectPtr<JxxClass_> x = new JxxClass_(std::forward<ARGS>(args)...);
                if (!x) return JS_INVALID_REFERENCE;
                JsValueRef out = JS_INVALID_REFERENCE;
                auto err = JsCreateExternalObjectWithPrototype(x.get(), JxxRelease, proto, &out);
                if (err) return JS_INVALID_REFERENCE;
                x.detach();
                return value_ref_t(out);
            }
        };
        template < class JxxClass_>
        static JxxObjectPtr<JxxClass_> TryGetAs(JsValueRef object) {
            IJxxObject* out = nullptr;
            auto err = JsGetExternalData(object, (void**)& out);
            if (err) return nullptr;
            return query_cast<JxxClass_>(out);
        };
    };

    using ExtObject = base_value_<ext_object_accessor>;

}; // namespace js