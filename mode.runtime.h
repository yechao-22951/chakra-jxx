#pragma once
#include "jxx.cxx.h"
#include "jxx.context.h"
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <bitset>

namespace mode {

	using namespace js;


	enum ModuleKind {
		BuildinModule,
		NativeModule,
		SourceModule,
	};

	struct BiModuleDesc {
		JxxCharPtr	path;		// internal/process.js
		JxxCharPtr name;		// process
		Object	exports;	// exports_;
	};

	class Runtime;
	struct ModuleControl {
		Runtime* runtime_;
		std::bitset<128>	priviliges_;
	};

	class Runtime : public JxxClassTemplate<Runtime, IJxxNativeObject> {
	public:
		using FilePath = std::filesystem::path;
		using WideString = std::wstring;
		struct ModuleRecord {
			ModuleKind	kind_;
			WideString	default_name_;
			FilePath	file_path_;
			Object		exports_;
		};
		using ActiveModuleMgr = std::unordered_map<FilePath, ModuleRecord>;
		using ModulePathes = std::vector<FilePath>;
		using BuildinModules = std::unordered_map<WideString, ModuleRecord>;
	protected:
		App& app_;
		JsRuntimeHandle		runtime_;
		ActiveModuleMgr		modules_;
		ModulePathes		search_dirs_;
		BuildinModules		buildin_modules_;
		Object				global_template_;	// need ref
		Context				genesis_context_;	// need ref
		std::bitset<128>	priviliges_;
	protected:
		static void CALLBACK __PromiseContinuation(JsValueRef task, void* this_) {
			((Runtime*)this_)->PromiseContinuation(task);
		}
		void PromiseContinuation(JsValueRef task) {
			// post to event loop
			JsAddRef(task, nullptr);
			auto cxx_task = [task]() {
				ContextScope scope(Context::From(task));
				if (scope.IsEntered()) {
					JsCallFunction(task, 0, 0, 0);
				}
				JsRelease(task, nullptr);
			};
		}
	public:
		FREE_IS_DELETE_THIS();

		Runtime(App& app, JsRuntimeAttributes attrs, JsThreadServiceCallback jtsc) : app_(app) {
			JsRuntimeHandle handle_ = JS_INVALID_REFERENCE;
			auto err = JsCreateRuntime(attrs, jtsc, &handle_);
			if (err) return;
			search_dirs_.push_back(std::filesystem::current_path());
			search_dirs_.push_back(std::filesystem::current_path().concat("node_modules"));
			genesis_context_ = Context::New(handle_);
			//JsSetRuntimeBeforeCollectCallback(handle_, this, nullptr);
			runtime_ = handle_;
		}
		~Runtime() {
			if (runtime_)
				JsDisposeRuntime(runtime_);
		}

		Context GenesisContext() {
			return genesis_context_;
		}

		static Runtime* GetFromContext(Context ctx) {
			return (Runtime*)ctx.GetData();
		}

		Context CreateContext(bool use_global_template = true) {
			Context context = Context::New(runtime_);
			if (!context) return context;
			auto err = context.SetData(this);
			if (context && use_global_template && global_template_) {
				ContextScope scope(context);
				Object global = context.Global();
				if (!global || global.SetPrototype(global_template_)) {
					context.reset();
				}
			}
			return context;
		}

		JsErrorCode RegisterGlobalTemplate(JsValueRef global) {
			return IF_EXCEPTION_RETURN(JsErrorInvalidArgument) = WHEN_RUN_{
				Object global_(global);
				if (!global_) return JsErrorInvalidArgument;
				global_template_ = global;
				return JsNoError;
			};
		}

		JsErrorCode RegisterModule(const BiModuleDesc& desc) {
			return IF_EXCEPTION_RETURN(JsErrorInvalidArgument) = WHEN_RUN_{
				if (!desc || !desc->name || !desc->exports)
					return JsErrorInvalidArgument;
				std::wstring name = desc->name;
				auto it = buildin_modules_.find(name);
				if (it != buildin_modules_.end())
					return JsErrorInvalidArgument;
				ModuleRecord record{
					BuildinModule,
					desc->name,
					desc->path ? desc->path : L"",
					desc->exports
				};
				buildin_modules_[std::move(name)] = std::move(record);
				return JsNoError;
			};
		}

		JsErrorCode LoadModule() {
			Context moduleContext = CreateContext(true);
		}
	};

	void InitMyRuntime() {
		Runtime* rt = new Runtime();
		Context genesis = rt->GenesisContext();
		// JxxGetGenesisContext();
		{
			ContextScope init_time(genesis);
			Object mConsole = JxxConsole::NewAsModule();
			Object mProcess = JxxProcess::NewAsModule();
			Object mFileSys = JxxFileSys::NewAsModule();
			Object mZlib = JxxZlib::NewInstance();
			Object mHttp = JxxHttp::NewInstance();
			rt->RegisterModule(BiModuleDesc{ L"internals/console", L"console", mConsole });
			rt->RegisterModule(BiModuleDesc{ L"internals/process", L"process", mProcess });
			rt->RegisterModule(BiModuleDesc{ L"internals/fs", L"fs", mFileSys });
			rt->RegisterModule(BiModuleDesc{ L"internals/zlib", L"zlib", mZlib });
			rt->RegisterModule(BiModuleDesc{ L"internals/fs", L"http", mHttp });
			Object global_template = Object::New();
			global_template[L"console"] = mConsole;
			global_template[L"process"] = mProcess;
			global_template[L"require"] = mConsole;
			rt->RegisterGlobalTemplate(global_template);
			// JxxRegisterGlobalTemplate(
		}
		Context main = rt->CreateContext(true);	// will use GlobalTemplate
		// JxxCreateContext
		{
			ContextScope main_context(main);
			rt->LoadModule(L"./app.js");		// let's go
		}
	}



};