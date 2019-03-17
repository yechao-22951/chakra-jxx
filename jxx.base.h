#pragma once
#include <ChakraCore.h>
#include "js.base.h"

#ifdef USE_JXX
#define JXXAPI extern
#else
#define JXXAPI __declspec(dllexport)
#endif

namespace js {

	using JXX_STR = WideCharPtr;
	using JXX_NAME = JXX_STR;
	using JXX_COUNT = size_t;
	using JXX_CALLEE = JsNativeFunction;
	using JXX_UINT = unsigned int;
	using JXX_BOOL = bool;

	struct JXX_CLASS_DEFINION;
	using JXX_CLASS_ID = void*;

	template <typename CXX> JXXAPI JXX_CLASS_DEFINION JXX_DEFINITION_OF_();

#define jxx_clsid_of_(X) ((JXX_DEFINITION_OF_<X>))
#define jxx_clsdef_of_(X) (jxx_clsid_of_(X)())

	struct JXX_PARENTS {
		JXX_CLASS_ID* ClassIDs;
		size_t Count;
	};

	struct JXX_EXPORT {
		JXX_NAME Name;
		JXX_CALLEE Callee;
	};

	struct JXX_EXPORTS {
		JXX_COUNT Count;
		const JXX_EXPORT* Entries;
	};

	struct JXX_CLASS_DEFINION {
		JXX_CLASS_ID ClassId;
		JXX_NAME	UncName;
		JXX_EXPORTS Methods;
		JXX_EXPORTS Functions;
		JXX_PARENTS Parents;
	};

	enum JXX_MIXIN_OPTIONS {
		MIXIN_METHOD = 1,			// class method
		MIXIN_FUNCTION = 2,			// class static function
	};

	using JxxErrorCode = error_t;

	class IJxxObject {
	public:
		static JXX_EXPORTS __JS_METHODS__() { return {}; };
		static JXX_EXPORTS __JS_FUNCTIONS__() { return {}; };
		static JXX_PARENTS __JS_PARENTS__() { return {}; };
	public:
		virtual ~IJxxObject() = 0;
		virtual refcnt_t AddRef() = 0;
		virtual refcnt_t Release() = 0;
		virtual void* QueryClass(JXX_CLASS_ID clsid) {
			if (clsid == jxx_clsid_of_(IJxxObject))
				return this;
			return nullptr;
		}
	};

	JXXAPI JXX_CLASS_DEFINION JxxQueryClass(JXX_CLASS_ID clsid) {
		using func_t = JXX_CLASS_DEFINION(*)();
		return ((func_t)clsid)();
	}

};