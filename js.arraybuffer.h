#pragma once

#include "js.object.h"
#include "js.typedarray.h"

namespace js {
    class arraybuffer_accessor_ ;
    using ArrayBuffer = base_value_<arraybuffer_accessor_>;

    class arraybuffer_accessor_ : public object_accessor_<_Buffer> {
    public:
        content_t GetContent() {
            content_t out = {};
            JsGetArrayBufferStorage(get(), &out.data, &out.size);
            return out;
        }

    public:
        static ArrayBuffer Alloc(length_t size) {
            value_ref_t out;
            JsCreateArrayBuffer(size, out.addr());
            return out;
        }
        static ArrayBuffer Attach(void* data, length_t size,
            JsFinalizeCallback do_free, void* cookie) {
            value_ref_t out;
            JsCreateExternalArrayBuffer(data, size, do_free, cookie, out.addr());
            return out;
        }
        static ArrayBuffer CreateFrom(String str) {
            uint32_t len = (uint32_t)str.size();
            char* ptr = new char[len + 1];
            if (!ptr) return value_ref_t();
            memcpy(ptr, str.c_str(), len + 1);
            value_ref_t out;
            JsCreateExternalArrayBuffer(ptr, len, free, ptr, out.addr());
            return out;
        }
    };

    

    class dataview_accessor_ : public object_accessor_<_DataView> {
    public:
        content_t GetContent() {
            content_t out = {};
            JsGetDataViewStorage(get(), &out.data, &out.size);
            return out;
        }

    public:
        static value_ref_t Create(ArrayBuffer buffer, length_t offset,
            length_t size) {
            value_ref_t out;
            JsCreateDataView(buffer, offset, size, out.addr());
            return out;
        }
    };

    using DataView = base_value_<dataview_accessor_>;

    content_t GetContent(JsValueRef ref) {
        value_ref_t x(ref);
        if (x.is(JsArrayBuffer)) {
            return ArrayBuffer(x).GetContent();
        }
        if (x.is(JsDataView)) {
            return DataView(x).GetContent();
        }
        if (x.is<JsTypedArray>()) {
            return TypedArray(x).GetContent();
        }
        return {};
    }

}; // namespace js