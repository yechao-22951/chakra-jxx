#pragma once

#include "jxx.cxx.h"

namespace mode {

using namespace js;

class WinHandle : public JxxClassTemplate<WinHandle, IJxxNativeObject> {
  protected:
    HANDLE handle_ = nullptr;

  public:
    virtual void Free() {
        if (!handle_)
            return;
        if (INVALID_HANDLE_VALUE == handle_)
            return;
        ::CloseHandle(handle_);
        handle_ = nullptr;
    }
    value_ref_t close() {
        if (!handle_)
            return JustIs(true);
        if (INVALID_HANDLE_VALUE == handle_)
            return JustIs(true);
        bool ok = ::CloseHandle(handle_) != FALSE;
        handle_ = nullptr;
        return JustIs(ok);
    }
};

_as_the<_Object> os_open(_as_the<_String> path, _as_the<_String> mode) {}

_as_the<_Object> os_read(_as_the<_String> path, _as_the<_String> mode) {}

_as_the<_Object> os_write(_as_the<_String> path, _as_the<_String> mode) {}

}; // namespace mode