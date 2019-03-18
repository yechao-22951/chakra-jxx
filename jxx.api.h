#pragma once
#include <ChakraCore.h>
#include <stdint.h>

#ifdef USE_JXX
#define JXXAPI extern
#else
#define JXXAPI __declspec(dllexport)
#endif

using JxxCharPtr = const char *;
using JxxNamePtr = JxxCharPtr;
using JxxCount = size_t;
using JxxFunction = JsNativeFunction;
using JxxUInt = uint32_t;
using JxxBool = bool;
using JxxErrorCode = int;
using JxxRefCount = uint32_t;
using JxxClassId = void *;

class JxxRuntime;

JXXAPI JxxRuntime *JxxCreateRuntime(JsRuntimeAttributes attr,
                                    JsThreadServiceCallback jts);

JXXAPI JxxRuntime *JxxGetCurrentRuntime();

JXXAPI JsContextRef JxxCreateContext(JxxRuntime *runtime);

JXXAPI JsValueRef JxxGetString(JxxCharPtr ptr, size_t len = 0);

JXXAPI JsValueRef JxxQueryProto(JxxCharPtr ptr);

JXXAPI JsValueRef JxxRegisterProto(JxxCharPtr ptr, JsValueRef proto);

JXXAPI JsValueRef JxxGetSymbol(JxxCharPtr ptr);

struct JxxParents {
    size_t Count;
    JxxClassId *ClassIDs;
};

struct JxxExport {
    JxxNamePtr Name;
    JxxFunction Callee;
};

struct JxxExports {
    JxxCount Count;
    const JxxExport *Entries;
};

struct JxxClassDefinition {
    JxxClassId ClassId;
    JxxNamePtr UncName;
    JxxExports Methods;
    JxxExports Functions;
    JxxParents Parents;
};

template <typename CXX> JXXAPI JxxClassDefinition JXX_DEFINITION_OF_() {
    static const JxxClassDefinition out = {
        (JxxClassId)&JXX_DEFINITION_OF_<CXX>, CXX::__JS_UNCNAME__,
        CXX::__JS_METHODS__(), CXX::__JS_FUNCTIONS__(), CXX::__JS_PARENTS__()};
    return out;
}

#define jxx_clsid_of_(X) ((JXX_DEFINITION_OF_<X>))
#define jxx_clsdef_of_(X) (jxx_clsid_of_(X)())

enum JxxMixinOptions {
    MIXIN_METHOD = 1,   // class method
    MIXIN_FUNCTION = 2, // class static function
};

#define DEFINE_CLASS_NAME(name) static inline JxxNamePtr __JS_UNCNAME__ = #name;
#define NO_CLASS_NAME() static inline JxxNamePtr __JS_UNCNAME__ = nullptr;

class IJxxObject {
  public:
    NO_CLASS_NAME();
    static JxxExports __JS_METHODS__() { return {}; };
    static JxxExports __JS_FUNCTIONS__() { return {}; };
    static JxxParents __JS_PARENTS__() { return {}; };

  public:
    virtual ~IJxxObject(){};
    virtual JxxRefCount AddRef() = 0;
    virtual JxxRefCount Release() = 0;
    virtual void *QueryClass(JxxClassId clsid) {
        if (clsid == &JXX_DEFINITION_OF_<IJxxObject>)
            return this;
        return nullptr;
    }
};

JXXAPI JxxClassDefinition JxxQueryClass(JxxClassId clsid);
JXXAPI int JxxMixinObject(JsValueRef object, JxxClassId clsid,
                          int MixinOptions);