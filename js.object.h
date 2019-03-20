#pragma once
#include "js.base.h"
#include "js.primitive.h"
#include "js.value.h"

namespace js {

    using propid_t = value_ref_t;
    using symbol_t = value_ref_t;

    propid_t PropertyId(const CharPtr name) {
        JsValueRef out;
        JsCreatePropertyId(name, strlen(name), &out);
        return out;
    }

    propid_t PropertyId(const String& name) {
        JsValueRef out;
        JsCreatePropertyId(name.c_str(), name.size(), &out);
        return out;
    }
    propid_t PropertyId(const StringView& name) {
        JsValueRef out;
        JsCreatePropertyId(name.data(), name.size(), &out);
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

        class Property {
        protected:
            value_ref_t this_;
            propid_t prop_id_;

        public:
            Property() {}
            Property(value_ref_t target, propid_t prop)
                : this_(target), prop_id_(prop) {}
            void operator=(const value_ref_t& val) {
                JSERR_TO_EXCEPTION(JsSetProperty(this_, prop_id_, val, true));
            }
            template <typename T>
            operator base_value_<T>() const {
                value_ref_t out;
                JsGetProperty(this_, prop_id_, out.address());
                if (out.is(JsUndefined))
                    return value_ref_t(JS_INVALID_REFERENCE);
                return out;
            }
            operator value_ref_t() const {
                value_ref_t out;
                JsGetProperty(this_, prop_id_, out.address());
                if (out.is(JsUndefined))
                    return JS_INVALID_REFERENCE;
                return out;
            }
        };

        Property operator[](const __prototype__& prop_id) {
            Property X(*this, PropertyId("__proto__"));
            return X;
        }

        Property operator[](propid_t prop_id) { return Property(*this, prop_id); }

        template <typename T> Property operator[](const T& x) {
            return Property(*this, PropertyId(x));
        }

        bool SetProperty(propid_t pid, const value_ref_t& v) {
            return JsSetProperty(*this, pid, v, false) == JsNoError;
        }
        bool SetProperty(const char* name, const value_ref_t& v) {
            return SetProperty(PropertyId(name), v) == JsNoError;
        }
        bool SetProperty(const String& name, const value_ref_t& v) {
            return SetProperty(PropertyId(name), v) == JsNoError;
        }
        bool SetProperty(const StringView& name, const value_ref_t& v) {
            return SetProperty(PropertyId(name), v) == JsNoError;
        }


        value_ref_t GetProperty(propid_t pid) const {
            value_ref_t out;
            JsGetProperty(*this, pid, out.address());
            if (out.is(JsUndefined))
                return JS_INVALID_REFERENCE;
            return out;
        }
        value_ref_t GetProperty(const char* name) const {
            propid_t pid = PropertyId(name);
            return GetProperty(pid);
        }
        value_ref_t GetProperty(const String& name)const {
            propid_t pid = PropertyId(name);
            return GetProperty(pid);
        }

        value_ref_t GetPrototype() const {
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
