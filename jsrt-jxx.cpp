// jsrt-js.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "js.value.h"
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include "jxx.class.h"
#include "jxx.runtime.h"

js::value_ref_t jello() {
	return nullptr;
}

class Console : public JxxClassTemplate<Console,IJxxObject> {
public:
	DEFINE_CLASS_NAME("org.mode.buildin.console");
public:
	js::value_ref_t log( js::more_list_ args ) {
		return JS_INVALID_REFERENCE;
	}
public:
	static inline const JXX_VIRTUAL_POINT __jxx__log = 
        Console::ADD_EXPORT_METHOD("log",(JxxFunction)&_STUB_OF_MAGIC_METHOD_OF_<&Console::log,js::DenyNew>);
};

int main()
{
	JxxRuntime runtime(JsRuntimeAttributeNone, nullptr);
	js::Context context = JxxCreateContext(&runtime);
	js::Context::Scope scope(context);
	js::value_ref_t str1 = JxxGetString("hello");
	js::value_ref_t str2 = JxxGetString("hello");
	js::value_ref_t str3 = js::just_is_("hello");
	js::value_ref_t sym1 = JxxGetSymbol("hello");
	js::Function js_jello = js::Function::FromMagic<jello>(nullptr, 0);
	JxxMixinObject(js_jello, jxx_clsid_of_(Console), MIXIN_METHOD);
	js_jello.Call(js::Undefined());
	js::ArrayBuffer buffer = js::ArrayBuffer::Alloc(1000);
	js::content_t content = buffer.GetContent();
	content[0] = '2';
	js::Durable<js::Object> hold(buffer);
	js::Function f = js::Function::FromMagic<jello>(nullptr, 0);
	f.Call(buffer);
	std::cout << "Hello World!\n";
}
