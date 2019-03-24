#pragma once

#include "js.object.h"
#include "js.typedarray.h"

namespace js {
    class arraybuffer_accessor_;
    using ArrayBuffer = base_value_<arraybuffer_accessor_>;

    /**
     * @brief Acceseor of JS ArrayBuffer.
     *
     */
    class arraybuffer_accessor_ : public object_accessor_<_Buffer> {
    public:
        /**
         * @brief Get the content(buffer) of the ArrayBuffer
         *
         * @return content_t The view of the ArrayBuffer's data.
         */
        content_t GetContent() {
            content_t out = {};
            JsGetArrayBufferStorage(get(), &out.data, &out.size);
            return out;
        }

    public:
        /**
         * @brief Alloc a ArrayBuffer with special size in bytes.
         *
         * @param size The ArrayBuffer size in bytes.
         * @return ArrayBuffer The ArrayBuffer object, it is JS_INVALID_REFERENCE if failed.
         */
        static ArrayBuffer Alloc(length_t size) {
            value_ref_t out;
            JsCreateArrayBuffer(size, out.addr());
            return out;
        }
        /**
         * @brief Create an external ArrayBuffer.
         *
         * @param data The poniter of the data.
         * @param size The size of the data.
         * @param do_free The destory function for data.
         * @param cookie The void * user data, will be passed to [do_free]
         * @return ArrayBuffer The ArrayBuffer object, it is JS_INVALID_REFERENCE if failed.
         */
        static ArrayBuffer Attach(void* data, length_t size,
            JsFinalizeCallback do_free, void* cookie) {
            value_ref_t out;
            JsCreateExternalArrayBuffer(data, size, do_free, cookie, out.addr());
            return out;
        }
        /**
         * @brief Create ArrayBuffer from multi-byte string.
         *
         * @param str The string object.
         * @return ArrayBuffer The ArrayBuffer object, it is JS_INVALID_REFERENCE if failed.
         */
        static ArrayBuffer CreateFrom(StringView str) {
            uint32_t len = (uint32_t)str.size();
            char* ptr = new char[len + 1];
            if (!ptr) return value_ref_t();
            memcpy(ptr, str.c_str(), len + 1);
            value_ref_t out;
            JsCreateExternalArrayBuffer(ptr, len, free, ptr, out.addr());
            if (!out) free(ptr);
            return out;
        }
        static ArrayBuffer CreateFrom(String str) {
            String pstr = new String(std::move(str));
            if (!pstr) return JS_INVALID_REFERENCE;
            value_ref_t out;
            JsCreateExternalArrayBuffer(
                pstr->data(),
                pstr->size(),
                [](void* p) {delete (String*)p; },
                pstr,
                out.addr()
            );
            if (!out) delete pstr;
            return out;
        }
        static ArrayBuffer CreateFrom(const StringView & str) {
            uint32_t len = (uint32_t)str.size();
            char* ptr = new char[len + 1];
            if (!ptr) return value_ref_t();
            memcpy(ptr, str.data(), len + 1);
            value_ref_t out;
            JsCreateExternalArrayBuffer(ptr, len, free, ptr, out.addr());
            return out;
        }
    };


    /**
     * @brief Acceseor of JS DataView.
     *
     */
    class dataview_accessor_ : public object_accessor_<_DataView> {
    public:
        /**
         * @brief Get the content(buffer) of the DataView
         *
         * @return content_t The view of the DataView's data.
         */
        content_t GetContent() {
            content_t out = {};
            JsGetDataViewStorage(get(), &out.data, &out.size);
            return out;
        }

    public:
        /**
         * @brief Create a JS DataView of the ArrayBuffer.
         *
         * @param buffer
         * @param offset
         * @param size
         * @return value_ref_t The DataView object.
         */
        static value_ref_t Create(ArrayBuffer buffer, length_t offset,
            length_t size) {
            value_ref_t out;
            JsCreateDataView(buffer, offset, size, out.addr());
            return out;
        }
    };

    using DataView = base_value_<dataview_accessor_>;

    /**
     * @brief Get the content of the ArrayBuffer-like object, include: JsArrayBuffer, JsDataView and JsTypedArray
     *
     * @param ref The target JS buffer-like object.
     * @return content_t The view of the ArrayBuffer's data.
     */
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