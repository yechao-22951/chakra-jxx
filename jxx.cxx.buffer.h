#pragma once
#include "jxx.cxx.h"
#include "jxx.var.primitive.h"
#include <memory>
#include <vector>

namespace jxx {

class Buffer : public JxxClassTemplate<Buffer> {
  public:
    virtual void Free() { delete this; }

  protected:
    using cxx_impl_t = std::vector<uint8_t>;
    cxx_impl_t data_;

  public:
    Value Alloc(_the<_Number> size, _the<_Number | _Optional> fill,
                _the<_String | _Optional> encoding) {
        Int i_size = get_as<Int>(size);
        if (!i_size)
            return Clear();
        if (i_size < 0)
            jxx_throw_error(ERR_MSG(JxxBufferSizeInvalid));
        data_.clear();
        data_ = std::move(cxx_impl_t(i_size, 0));
        return nullptr;
    }
    Value From(_the<_Buffer | _Array | _String | _Object> source,
               _the<_Number | _Optional> offset,
               _the<_Number | _Optional> size) {
        Int i_size = get_as<Int>(size);
        if (!i_size)
            return Clear();
        if (i_size < 0)
            jxx_throw_error(ERR_MSG(JxxBufferSizeInvalid));
        data_.clear();
        data_ = std::move(cxx_impl_t(i_size, 0));
        return nullptr;
    }
    Value Resize(_the<_Number> size) {
        Int i_size = get_as<Int>(size);
        if (!i_size)
            return Clear();
        if (i_size < 0)
            jxx_throw_error(ERR_MSG(JxxBufferSizeInvalid));
        data_.resize(i_size);
        return nullptr;
    }
    Value Clear() {
        data_.clear();
        return nullptr;
    }

  public:
    JXX_EXPORT_BY_NAME(Buffer, alloc, Alloc);
    JXX_EXPORT_BY_NAME(Buffer, resize, Resize);
    JXX_EXPORT_BY_NAME(Buffer, clear, Clear);
};

}; // namespace jxx