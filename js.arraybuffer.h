#pragma once

#include "js.object.h"
#include "js.typedarray.h"

namespace js {

    class arraybuffer_accessor_ : public object_accessor_<_Buffer> {
    public:
        content_t GetContent() {
            content_t out = {};
            JsGetArrayBufferStorage(get(), &out.data, &out.size);
            return out;
        }
    public:
        static value_ref_t Alloc(length_t size) {
            value_ref_t out;
            JsCreateArrayBuffer(size, out.addr());
            return out;
        }
        static value_ref_t Attach(void* data, length_t size, JsFinalizeCallback do_free, void* cookie) {
            value_ref_t out;
            JsCreateExternalArrayBuffer(data, size, do_free, cookie, out.addr());
            return out;
        }
    };

    using ArrayBuffer = base_value_<arraybuffer_accessor_>;

    class dataview_accessor_ : public object_accessor_<_DataView> {
    public:
        content_t GetContent() {
            content_t out = {};
            JsGetDataViewStorage(get(), &out.data, &out.size);
            return out;
        }
    public:
        static value_ref_t Create(ArrayBuffer buffer, length_t offset, length_t size) {
            value_ref_t out;
            JsCreateDataView(buffer, offset, size, out.addr());
            return out;
        }
    };

    using DataView = base_value_<dataview_accessor_>;

    content_t GetContent(JsValueRef ref) {
        value_ref_t x(ref);
        if (x.is<_Buffer>()) {
            return ArrayBuffer(x).GetContent();
        }
        if (x.is<_DataView>()) {
            return DataView(x).GetContent();
        }
        if (x.is<_TypedArray>()) {
            return TypedArray(x).GetContent();
        }
        return {};
    }

};