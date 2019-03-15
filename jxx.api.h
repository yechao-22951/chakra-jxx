#pragma once
#include <ChakraCore.h>

#ifdef USE_JXX
#define JXXAPI extern
#else
#define JXXAPI __declspec(dllexport)
#endif

#define JXX_HIDDEN_ME enum { __VISIBILITY__ = false };

using JXX_NAME = const wchar_t *;
using JXX_STR = const wchar_t * ;
using JXX_COUNT = size_t;
using JXX_CALLEE = JsNativeFunction;
using JXX_MIXIN = int (*)(void *);
using JXX_QUERY_SUPER = void *(*)(void *);
using JXX_UINT = unsigned int;
using JXX_BOOL = bool;

struct JXX_CLASS_DEFINION;
using JXX_CLASS_ID = void *;
// typedef JXX_CLASS_DEFINION(*JXX_CLASS_ID)();

template <typename CXX> JXXAPI JXX_CLASS_DEFINION JXX_DEFINITION_OF_();

#define jxx_clsid_of_(X) ((JXX_DEFINITION_OF_<X>))
#define jxx_clsdef_of_(X) (jxx_clsid_of_(X)())

struct JXX_PARENTS {
    JXX_CLASS_ID *ClassIDs;
    size_t Count;
};
struct JXX_EXPORT {
    JXX_NAME Name;
    JXX_CALLEE Callee;
    JXX_COUNT ArgumentCount;
};
struct JXX_EXPORTS {
    JXX_COUNT Count;
    const JXX_EXPORT *Entries;
};
struct JXX_CLASS_DEFINION {
    JXX_CLASS_ID ClassId;
    const char *Name;
    JXX_EXPORTS Methods;
    JXX_EXPORTS Functions;
    JXX_PARENTS Parents;
};

using JxxErrorCode = size_t;
using JxxClassId = JXX_CLASS_ID;
using JxxErrorCode = size_t;

enum {
    JxxErrorTypeMismatch = -1,
    JxxBufferSizeInvalid = -2,
    JxxOutOfMemory = -3,
    JxxErrorMixin = -4,
};

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define ERR_MSG(x) x, __FILE__ ":" STRINGIFY(__LINE__) " : " #x

class IJxxNativeObject {
  public:
    static JXX_EXPORTS __JS_METHODS__() { return {}; };
    static JXX_EXPORTS __JS_FUNCTIONS__() { return {}; };
    static JXX_PARENTS __JS_PARENTS__() { return {}; };

  public:
    void *_query_service(JxxClassId clsid) {
        if (clsid == jxx_clsid_of_(IJxxNativeObject))
            return this;
        return nullptr;
    }

  public:
    virtual long AddRef() = 0;
    virtual long Release() = 0;
    virtual void *QueryService(JxxClassId clsid) = 0;
    virtual void Free() = 0;
};

JXXAPI JsValueRef JxxException(JxxErrorCode err, const char *errmsg) {
    // throw std::exception(errmsg);
    return nullptr;
}

JXXAPI JXX_CLASS_DEFINION JxxQueryClass(JxxClassId clsid) {
    using func_t = JXX_CLASS_DEFINION (*)();
    return ((func_t)clsid)();
}

JXXAPI void CHAKRA_CALLBACK JxxNativeObjectRelease(void *p) {
    if (!p)
        return;
    ((IJxxNativeObject *)p)->Release();
}
JXXAPI void JxxNativeObjectReference(IJxxNativeObject *p) {
    if (!p)
        return;
    p->AddRef();
}
//
//JXXAPI JsValueRef JxxExceptionCreate(  ) { return nullptr; };
//JXXAPI long JxxMixinObject(JsValueRef object, JXX_CLASS_ID clsid);
//
//
//JXXAPI long JxxCreateRuntime(  );
//JxxRegisterBuildin(LPCWSTR lpName, JsValueRef x);
//
//struct _JxxModule {
//
//};
//
//JxxApp.RegisterRuntimeBootstrap(  );
//
//main() {
//	IJxxRuntime * rt = JxxCreateRuntime();
//	JsValueRef console = JxxCreateNativeObject(jxx_clsid_of(JxxConsole));
//	JsValueRef process = JxxCreateNativeObject(jxx_clsid_of(JxxProcess));
//	JsValueRef process = JxxCreateNative(jxx_clsid_of(JxxProcess));
//	////////////////////////////////////////////////////////////////////
//	JxxRegisterModule(rt, L"console", console);		// 
//	JxxRegisterModule(rt, L"console", console);		// 
//	JxxRegisterModule(rt, L"console", console);		// 
//	JxxRegisterModule(rt, L"console", console);		// 
//	JxxRegisterModule(rt, L"fs", console);			// 
//	JxxRegisterModule(rt, L"net", console);			// 
//	JxxRegisterModule(rt, L"http", console);		// 
//	JxxRegisterModule(rt, L"zlib", zlib);			// 
//	JxxRegisterGlobalPrototype(rt globalTemplate);	// ����ȫ�ֶ���ԭ��
//
//
//	JxxLoadModule();
//	JxxLoadNativeModule();		// JxxModuleInit( JsContext * );
//
//	js::Object globalTemplate;
//	globalTemplate[L"console"] = console.exports;	//
//	globalTemplate[L"process"] = process.exports;
//	globalTemplate[L"require"] = core.require;		// 
//
//	// ʹ��Jxx����Context��Context���Զ��̳�ȫ��ԭ��
//	JxxCreateContext() {
//		JsCreateContext();
//		JsGetGlobalObject();
//		JsSetPrototype(globalTemplate);
//	}
//
//	// ���Զ�����JsRuntime�󶨵�JxxRuntime����
//	JxxCreateRuntime() {
//	
//	}
//
//	JxxApp*;					// only one App instance to manager process-level data, like 
//	JxxRuntime*;
//	JxxContext*;				//
//	JokaPostMessage;			// topic, message/
//	Joka
//	// JxxRuntime �߱���
//	// ģ���б����á��ⲿ��		����֧�� require
//	// Contextȫ�ֶ����ԭ��		
//	// Promise����				����֧�� async/await
//	// ...
//
//}