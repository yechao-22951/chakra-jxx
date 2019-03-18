#pragma once
#include "jxx.api.h"
#include "jxx.cc.h"
#include "js.context.h"
#include "jxx.class.h"

class JxxRuntime : public JxxClassTemplateNE<JxxRuntime, IJxxObject> {
protected:
	js::cache_t	cache_;
	js::Runtime runtime_;
public:
	JxxRuntime(JsRuntimeAttributes attr, JsThreadServiceCallback jts) : runtime_(attr, jts) {
	}
	~JxxRuntime() {
		cache_.clear();
	}
public:
	js::cache_t& cache() { return cache_; };
	JsRuntimeHandle handle() { return runtime_; };
};


JXXAPI JsContextRef JxxCreateContext(JxxRuntime* runtime) {
	return js::Context::Create(runtime->handle(), runtime);
}

JXXAPI JsValueRef JxxGetString(const char* ptr, size_t len) {
	js::Context context = js::Context::Current();
	if (!context) return JS_INVALID_REFERENCE;
	JxxRuntime* rt = (JxxRuntime*)context.GetData();
	if (!rt) return JS_INVALID_REFERENCE;
	len = len ? len : strlen(ptr);
	return rt->cache().get_string(std::move(std::string(ptr, len)));
}

JXXAPI JsValueRef JxxQueryProto(const char* ptr) {
	js::Context context = js::Context::Current();
	if (!context) return JS_INVALID_REFERENCE;
	JxxRuntime* rt = (JxxRuntime*)context.GetData();
	if (!rt) return JS_INVALID_REFERENCE;
	return rt->cache().get_proto(std::move(std::string(ptr)));
}

JXXAPI JsValueRef JxxRegisterProto(const char* ptr, JsValueRef proto) {
	js::Object prototype(proto);
	if (!prototype) return JS_INVALID_REFERENCE;
	js::Context context = js::Context::Current();
	if (!context) return JS_INVALID_REFERENCE;
	JxxRuntime* rt = (JxxRuntime*)context.GetData();
	if (!rt) return JS_INVALID_REFERENCE;
	rt->cache().put_proto(std::string(ptr), prototype);
	return proto;
}

JXXAPI JsValueRef JxxGetSymbol(const char* ptr) {
	js::Context context = js::Context::Current();
	if (!context) return JS_INVALID_REFERENCE;
	JxxRuntime* rt = (JxxRuntime*)context.GetData();
	if (!rt) return JS_INVALID_REFERENCE;
	return rt->cache().get_symbol(std::move(std::string(ptr)));
}