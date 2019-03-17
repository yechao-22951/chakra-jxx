// jsrt-js.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "js.value.h"
#include "js.arraybuffer.h"
#include "js.context.h"
#include "js.function.h"
#include <experimental/coroutine>

js::value_ref_t jello() {
	return nullptr;
}

int main()
{
	js::Runtime runtime(JsRuntimeAttributeNone, nullptr);
	js::Context context = js::Context::Create(runtime, 0);
	js::Context::Scope scope(context);
	js::ArrayBuffer buffer = js::ArrayBuffer::Alloc(1000);
	js::content_t content = buffer.GetContent();
	content[0] = '2';
	js::Durable<js::Object> hold(buffer);
	js::Function f = js::Function::FromMagic<jello>(nullptr, 0);
	f.Call(buffer);
	std::cout << "Hello World!\n";
}
