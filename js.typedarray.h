#pragma once
#include "js.array.h"

namespace js {

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

    template <typename ElementType_> struct array_type_of_native_;

    template <> struct array_type_of_native_<int8_t> {
        enum { mask = 1 << JsArrayTypeInt8 };
    };
    template <> struct array_type_of_native_<uint8_t> {
        enum { mask = (1 << JsArrayTypeUint8) | (1 << JsArrayTypeUint8Clamped) };
    };
    template <> struct array_type_of_native_<int16_t> {
        enum { mask = 1 << JsArrayTypeInt16 };
    };
    template <> struct array_type_of_native_<uint16_t> {
        enum { mask = 1 << JsArrayTypeUint16 };
    };
    template <> struct array_type_of_native_<int32_t> {
        enum { mask = 1 << JsArrayTypeInt32 };
    };
    template <> struct array_type_of_native_<uint32_t> {
        enum { mask = 1 << JsArrayTypeUint32 };
    };
    template <> struct array_type_of_native_<float> {
        enum { mask = 1 << JsArrayTypeFloat32 };
    };
    template <> struct array_type_of_native_<double> {
        enum { mask = 1 << JsArrayTypeFloat64 };
    };

    class typed_array_accessor_;

    class MemoryView {
        friend class typed_array_accessor_;
    protected:
        value_ref_t    array_;
        int            el_size_;
        content_t    content_;
        JsTypedArrayType el_type_;
    public:
        size_t Length() {
            return content_.size / el_size_;
        }
        template <typename T>
        bool IsFit() {
            return (array_type_of_native_<T>::mask & (1 << el_type_)) != 0;
        }
        //template <typename T>
        //bool GetElement( size_t index, T & out ) {
        //    if( !IsFit<T>() ) return false;

        //}
        //template <typename T>
        //bool SetElement(size_t index, T& out) {

        //}
        template < typename T >
        T& operator [] (size_t index) {
            size_t offset = index * el_size_;
            CXX_EXCEPTION_IF(ErrorInvalidArrayIndex, offset + el_size_ > content_.size);
            return *(T*)(content_.data + offset);
        }
    };

    class typed_array_accessor_ : public object_accessor_<_TypedArray> {
    public:
        MemoryView GetView() {
            MemoryView view = {};
            auto err = JsGetTypedArrayStorage(
                get(), &view.content_.data,
                &view.content_.size, &view.el_type_,
                &view.el_size_);
            if (err) return {};
            view.array_ = get();
            return view;
        }
        content_t GetContent() {
            auto mv = GetView();
            return mv.content_;
        }
    };

    using TypedArray = base_value_< typed_array_accessor_ >;



};