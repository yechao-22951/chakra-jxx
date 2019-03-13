#pragma once

#include "jxx.cxx.h"

namespace mode {

using namespace jxx;

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
    js_value_t close() {
        if (!handle_)
            return just_is_(true);
        if (INVALID_HANDLE_VALUE == handle_)
            return just_is_(true);
        bool ok = ::CloseHandle(handle_) != FALSE;
        handle_ = nullptr;
        return just_is_(ok);
    }
};

_the<_Object> os_open(_the<_String> path, _the<_String> mode) {}

_the<_Object> os_read(_the<_String> path, _the<_String> mode) {}

_the<_Object> os_write(_the<_String> path, _the<_String> mode) {}

}; // namespace mode