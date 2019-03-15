#pragma once

#include "js.object.h"

namespace js {

	class arraybuffer_accessor_ : public object_accessor_<_Buffer> {
	public:
		content_t GetContent() {
			content_t out = {};
			JsGetArrayBufferStorage(get(), &out.data, &out.size);
			return out;
		}
	public:
		static value_ref_t Alloc(uint32_t size) {
			value_ref_t out;
			JsCreateArrayBuffer(size, out.addr());
			return out;
		}
		static value_ref_t Attach(void* data, uint32_t size, JsFinalizeCallback do_free, void* cookie) {
			value_ref_t out;
			JsCreateExternalArrayBuffer(data, size, do_free, cookie, out.addr());
			return out;
		}
	};

	using ArrayBuffer = base_value_<arraybuffer_accessor_>;

};