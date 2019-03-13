#pragma once
#include "jxx.var.object.h"
#include <string>

namespace jxx {

class buffer_accessor_ : public object_accessor_<_Buffer> {
  protected:
    ChakraBytePtr data_ = nullptr;
    unsigned int size_ = 0;

  public:
    bool IsShared() {}
    size_t Snap() {
        ChakraBytePtr ptr = nullptr;
        unsigned int size = 0;
        auto err = JsGetArrayBufferStorage(get(), &ptr, &size);
        if (err)
            return 0;
        data_ = ptr;
        size_ = size;
        return size;
    }
    ChakraBytePtr Data() { return data_; }
    size_t Size() { return size_; }
    uint8_t &operator[](size_t index) { return data_[index]; }
    static js_value_t FromInterface(IJxxNativeBuffer *buffer) {
        if (!buffer)
            return nullptr;
        JsValueRef out = nullptr;
        auto err =
            JsCreateExternalArrayBuffer(buffer->GetBuffer(), buffer->GetSize(),
                                        JxxNativeObjectRelease, buffer, &out);
        if (!err)
            return nullptr;
        JxxNativeObjectReference(buffer);
        return out;
    }
    js_value_t CreateView(uint32_t offset, uint32_t size) {
        JsValueRef view = nullptr;
        auto err = JsCreateDataView(get(), offset, size, &view);
        if (err)
            return nullptr;
        return view;
    }
    js_value_t CreateTypedArray(uint32_t offset, uint32_t size) {
        JsValueRef view = nullptr;
        auto err = JsCreateTypedArray(get(), offset, size, &view);
        if (err)
            return nullptr;
        return view;
    }
    template <typename Element_>
    static js_value_t FromString(std::basic_string<Element_> str) {
        std::basic_string<Element_> *ptr =
            new std::basic_string<Element_>(std::move(str));
        if (!ptr)
            return nullptr;
        JsValueRef out = nullptr;
        void *data = ptr->data();
        uint32_t size = (uint32_t)ptr->size() * sizeof(Element_);
        auto err =
            JsCreateExternalArrayBuffer(data, size, CppDelete, ptr, &out);
        if (!err)
            return out;
        delete ptr;
        return nullptr;
    }

    template <typename Element_>
    static js_value_t FromView(const std::basic_string_view<Element_> &view) {
        JsValueRef out = nullptr;
        void *data = (void *)view->data();
        uint32_t size = (uint32_t)view->size() * sizeof(Element_);
        auto err =
            JsCreateExternalArrayBuffer(data, size, nullptr, nullptr, &out);
        if (err)
            return nullptr;
        return out;
    }

    static js_value_t FromPlainBuffer(void *buffer, uint32_t len,
                                      decltype(free) free_fn) {
        if (!buffer || !len)
            return nullptr;
        JsValueRef out = nullptr;
        auto err =
            JsCreateExternalArrayBuffer(buffer, len, free_fn, buffer, &out);
        if (!err)
            return nullptr;
        return out;
    }
};

using Buffer = value_as_<buffer_accessor_>;

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
    enum {
        type_mask = (1 << JsArrayTypeUint8) | (1 << JsArrayTypeUint8Clamped)
    };
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

}; // namespace jxx