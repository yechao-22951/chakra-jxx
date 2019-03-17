#pragma once
#include "js.base.h"
#include "js.primitive.h"
#include "js.object.h"
#include "js.function.h"
#include "jxx.base.h"
#include <memory>
#include <vector>
#include <atomic>
namespace js {

	using counter_t = std::atomic<uint32_t>;

	template <typename From_, typename To_> To_* query_cast(From_* from) {
		return from->QueryClass(jxx_clsid_of(To_));
	}

	template <typename JxxObject_> class JxxObjectPtr {
	protected:
		JxxObject_* nake_ = nullptr;
	public:
		JxxObject_* get() { return nake_; }
		JxxObjectPtr() = default;
		JxxObjectPtr(IJxxObject* p) { reset(p, true); }
		JxxObjectPtr(JxxObject_* ptr) { reset(ptr, true); }
		JxxObjectPtr(const JxxObjectPtr& r) { reset(r.get(), true); }
		JxxObjectPtr(JxxObjectPtr&& r) { nake_ = r.detach(); }
		template <typename K> JxxObjectPtr(const JxxObjectPtr<K>& r) { reset(r); }
		template <typename K> JxxObjectPtr(JxxObjectPtr<K>&& r) { reset(r); }
		//////////////////////////////////////////////////
		JxxObjectPtr& attach(JxxObject_* ptr) { return reset(ptr, false); }
		JxxObject_* detach() { return std::exchange(nake_, nullptr); }
		JxxObjectPtr& reset() {
			auto old = std::exchange(nake_, nullptr);
			if (old)
				old->Release();
			return *this;
		}
		JxxObjectPtr& reset(JxxObject_* ptr, bool ref) {
			auto old = std::exchange(nake_, ptr);
			if (ptr && ref)
				ptr->AddRef();
			if (old)
				old->Release();
			return *this;
		}
		template <typename K> JxxObjectPtr& reset(K* ptr, bool ref) {
			JxxObject_* np = query_cast(ptr);
			return reset(np, ref);
		}
		template <typename K> JxxObjectPtr& reset(const JxxObjectPtr<K>& ptr) {
			JxxObject_* np = query_cast(ptr.get());
			return reset(np, true);
		}
		template <typename K> JxxObjectPtr& reset(JxxObjectPtr<K>&& ptr) {
			JxxObject_* np = query_cast(ptr.get());
			if (np) ptr.detach();
			return reset(np, false);
		}
		//////////////////////////////////////////////////
		JxxObject_* operator->() { return nake_; }
		const JxxObject_* operator->() const { return nake_; }
		//////////////////////////////////////////////////
		operator bool() const { return nake_ != nullptr; }
	};

	struct JXX_VIRTUAL_POINT {};

	template <typename This_, typename... Implements_>
	class JxxClassTemplateNE : public Implements_... {
	public:
		static inline JXX_CLASS_ID __PARENTS__[] = {
			((JXX_CLASS_ID)JXX_DEFINITION_OF_<Implements_>)..., nullptr };
		static JXX_EXPORTS __JS_METHODS__() { return {}; }
		static JXX_EXPORTS __JS_FUNCTIONS__() { return {}; }
		static JXX_PARENTS __JS_PARENTS__() {
			return { __PARENTS__, sizeof...(Implements_) };
		};
	protected:
		counter_t ref_count_;
	public:
		virtual long AddRef() { return ++ref_count_; }
		virtual long Release() {
			auto c = --ref_count_;
			if (!c) { delete this; };
			return c;
		}
		virtual void* QueryClass(JXX_CLASS_ID clsid) {
			if (clsid == jxx_clsid_of_(This_)) return this;
			void* ptrs_[] = { (Implements_::QueryClass(clsid))..., 0 };
			for (size_t i = 0; i < sizeof...(Implements_) + 1; ++i) {
				auto ptr = ptrs_[i];
				if (ptr)
					return ptr;
			}
			return nullptr;
		}
	};

	template <typename This_, typename... Implements_>
	class JxxClassTemplate : public JxxClassTemplateNE<This_, Implements_...> {
	public:
		static JXX_EXPORTS __JS_METHODS__() {
			return JXX_EXPORTS{
				(JXX_COUNT)__EXPORTED_METHODS__.size(),
				__EXPORTED_METHODS__.empty()
				? nullptr
				: __EXPORTED_METHODS__.data() };
		}
		static JXX_EXPORTS __JS_FUNCTIONS__() {
			return JXX_EXPORTS{
				(JXX_COUNT)__EXPORTED_FUNCTIONS__.size(),
				__EXPORTED_FUNCTIONS__.empty()
				? nullptr
				: __EXPORTED_FUNCTIONS__.data() };
		}
	protected:
		static inline std::vector<JXX_EXPORT> __EXPORTED_METHODS__;
		static inline std::vector<JXX_EXPORT> __EXPORTED_FUNCTIONS__;
		static JXX_VIRTUAL_POINT ADD_EXPORT_METHOD(JXX_NAME Name, JXX_CALLEE JxxFunc) {
			__EXPORTED_METHODS__.push_back({ Name, JxxFunc });
			return JXX_VIRTUAL_POINT{};
		}
		static JXX_VIRTUAL_POINT ADD_EXPORT_FUNCTION(JXX_NAME Name, JXX_CALLEE JxxFunc) {
			__EXPORTED_FUNCTIONS__.push_back({ Name, JxxFunc });
			return JXX_VIRTUAL_POINT{};
		}
	private:
		template <typename Function_, std::size_t... I>
		void ____call_magic_method(Function_ function, call_info_t& info, std::index_sequence<I...>)
		{
			param_t params_[] = { (param_t(info, I))..., {} };
			This_* this_ = (This_*)this;
			info.returnValue = (this_->*fn)(params_[I]...);
		}

	public:
		template <auto Method_, int Deny_ = DenyNew>
		static JsValueRef CHAKRA_CALLBACK
			_STUB_OF_MAGIC_FUNC_OF_(JsValueRef callee,
				bool isNew,
				JsValueRef * arguments,
				unsigned short argumentsCount,
				void* jxxClassId)
		{
			return THROW_JS_EXCEPTION_(
				[&]() {
					static const std::size_t N = ARG_COUNT_OF_<decltype(Method_)>::value;
					CXX_EXCEPTION_IF(JsErrorInvalidArgument, argumentsCount == 0);
					int mode = isNew ? DenyNew : DenyNormal;
					CXX_EXCEPTION_IF(ErrorCallModeIsDenied, mode & Deny_);
					Object self(arguments[0]);
					CXX_EXCEPTION_IF(JsErrorInvalidArgument, !self);
					IJxxObject * JxxObject = (IJxxObject*)self.GetExtenalData();
					CXX_EXCEPTION_IF(JsErrorInvalidArgument, !JxxObject);
					This_ * JxxThis = (This_*)JxxObject->QueryClass(jxxClassId);
					CXX_EXCEPTION_IF(JsErrorInvalidArgument, !JxxThis);
					call_info_t info = { callee,
										isNew,
										arguments[0],
										arguments + 1,
										(size_t)(argumentsCount - 1),
										jxxClassId };
					auto I___ = std::make_index_sequence<N>{};
					JxxThis->____call_magic_method(Method_, info, I___);
					return info.returnValue;
				});
		};

	public:
		//// 创建一个实例，包含了一个JS扩展对象和一个C++对象
		//template <typename... ARGS>
		//static js::value_ref_t NewInstance(ARGS... args) {
		//	JxxObjectPtr<This_> ret(new This_(std::forward<ARGS&&>(args)...));
		//	if (!ret)
		//		return JS_INVALID_REFERENCE;
		//	js::Object instance = JxxCreateExObject(nullptr, ret.get());
		//	if (!instance)
		//		return instance;
		//	JxxMixinObject(instance, jxx_clsid_of_(This_));
		//	return instance;
		//}

		//static JsValueRef Mixin() {
		//	js::Object instance = js::ObjectCreate();
		//	JsValueRef out = nullptr;
		//	auto err = JsCreateObject(&out);
		//	if (err)
		//		return nullptr;
		//	JXX_CLASS_DEFINION def = JXX_DEFINITION_OF_<This_>();
		//	for (size_t i = 0; i < def.Exports.Count; ++i) {
		//		std::string name_a = def.Exports.Entries[i].Name;
		//		std::wstring name_w = std::wstring(name_a.begin(), name_a.end());
		//		JsPropertyIdRef pid = nullptr;
		//		err = JsGetPropertyIdFromName(name_w.c_str(), &pid);
		//		if (err)
		//			continue;
		//	}
		//}
	};

	template <typename CXX> JXXAPI JXX_CLASS_DEFINION JXX_DEFINITION_OF_() {
		static const JXX_CLASS_DEFINION out = {
			(JxxClassId)& JXX_DEFINITION_OF_<CXX>, typeid(CXX).name(),
			CXX::__JS_METHODS__(), CXX::__JS_FUNCTIONS__(), CXX::__JS_PARENTS__() };
		return out;
	}

	template <typename T>
	class JxxOf : public JxxClassTemplate<JxxOf<T>>, public T {
	public:
		T* get() { return this; };
	};

	template <typename T> void CHAKRA_CALLBACK CppDelete(void* ptr) {
		delete (T*)ptr;
	}

#define JXX_EXPORT_METHOD(K, CXX_NAME)                                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_METHOD(                                                  \
            L#CXX_NAME,           \
            (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_METHOD_RENAME(K, CXX_NAME, JS_NAME)                         \
    static inline const JXX_VIRTUAL_POINT __jxx__##JS_NAME =                   \
        K::ADD_EXPORT_METHOD(                                                  \
            L#JS_NAME,           \
            (JXX_CALLEE)&JXX_NATIVE_METHOD_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_FUNCTION(K, CXX_NAME)                                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            L#CXX_NAME,           \
            (JXX_CALLEE)&JXX_CXX_FUNCTION_OF_<&K::CXX_NAME>);

#define JXX_EXPORT_FUNCTION_RENAME(K, CXX_NAME, JS_NAME)                       \
    static inline const JXX_VIRTUAL_POINT __jxx__##CXX_NAME =                  \
        K::ADD_EXPORT_FUNCTION(                                                \
            L#JS_NAME,            \
            (JXX_CALLEE)&JXX_CXX_FUNCTION_OF_ & K::CXX_NAME);

	JXXAPI error_t JxxMixinObject(JsValueRef object, JXX_CLASS_ID clsid, int MixinOptions) {
		Object target(object);
		if (!target)
			return ErrorTypeMismatch;
		auto def = JxxQueryClass(clsid);
		for (size_t i = 0; i < def.Parents.Count; ++i) {
			auto err = JxxMixinObject(object, def.Parents.ClassIDs[i], MixinOptions);
			if (err < 0)
				return err;
		}
		if (MixinOptions & MIXIN_METHOD) {
			for (size_t i = 0; i < def.Methods.Count; ++i) {
				auto& item = def.Methods.Entries[i];
				JsValueRef name = just_is_(item.Name);
				if (!name)
					return JsErrorOutOfMemory;
				Function fn = Function::FromNativeFunction(item.Callee, name, clsid);
				if (!fn)
					return JsErrorOutOfMemory;
				target.SetProperty(make_prop_id(item.Name), fn);
			}
		}
		if (MixinOptions & MIXIN_FUNCTION) {
			for (size_t i = 0; i < def.Functions.Count; ++i) {
				auto& item = def.Functions.Entries[i];
				JsValueRef name = just_is_(item.Name);
				if (!name)
					return JsErrorOutOfMemory;
				Function fn = Function::FromNativeFunction(item.Callee, name, clsid);
				if (!fn)
					return JsErrorOutOfMemory;
				target.SetProperty(make_prop_id(item.Name), fn);
			}
		}
		return 0;
	}
};