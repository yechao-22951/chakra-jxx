#pragma once
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include "js.primitive.h"
#include "jxx.class.h"
#include "jxx.runtime.h"
#include <functional>

namespace win {

    using namespace js;

    _as_the<_Number> _RegCreateKey(
        _as_the<_Number> hkey,
        _as_the<_String> subkey,
        _as_the<_Object | _Optional> options)
    {
        HKEY h =(HKEY)(LONG_PTR)(GetAs<double>(hkey));
        std::string path = GetAs<String>(subkey);
        HKEY out = nullptr;
        CXX_EXCEPTION_IF(RegCreateKeyA(h, path.c_str(), &out));
        return Just<double>(out);
    }

};