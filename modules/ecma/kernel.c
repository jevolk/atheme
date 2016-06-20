/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct kernel
:dasein
{
	using task = vorhanden::task;

	void terminate(task &, v8::Local<v8::Value> retval = {});
	void handle_descriptor(task &, v8::Local<v8::Object> &value);
	void handle_result_object_generator(task &, v8::Local<v8::Object> &result);
	void handle_result_object_iterator(task &, v8::Local<v8::Object> &result);
	void handle_result_object(task &, v8::Local<v8::Object> &result);

	v8::Local<v8::Value> target_sys(task &, v8::Local<v8::Value> &result);
	void enter_user(task &, function::argv &);

	bool spawn_init(task &, const char *const &input);
	void spawn(sourceinfo_t *const &si, const char *const &input);

	void handle_hook(const char *const &name, void *const &data, hook_parser &parser);
	void target_hook(const char *const &name, void *const &data, hook_parser &parser) noexcept;

	void handle_input(sourceinfo_t *const &si, const char *const &input);
	void target_input(sourceinfo_t *const &si, const char *const &input) noexcept;

	void handle_ereignis();
	void target_ereignis() noexcept;

	kernel();
	~kernel();
}

static *kernel;

struct entry_scope
{
	struct isolate_scope isolate_scope;
	struct handle_scope handle_scope;
	struct context_scope context_scope {*kernel};
};


DECLARE_MODULE_V1
(
	"ecma/kernel",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		const isolate_scope isolate_scope;
		const handle_scope handle_scope;
		kernel = new struct kernel;

		const context_scope context_scope(*kernel);
		auto proc(get<v8::Object>("proc"));
		set(proc, 0, LS1::external(kernel));
	},
	[](module_unload_intent_t)
	{
		const context_scope context_scope(*kernel);
		auto proc(get<v8::Object>("proc"));
		del(proc, 0);
	},
	PACKAGE_STRING,
	"jzk"
);


kernel::kernel()
:dasein{ecma->welt}
{
	ecma->star = std::bind(&kernel::target_sys, this, ph::_1, ph::_2);
	ecma->hookv = std::bind(&kernel::target_hook, this, ph::_1, ph::_2, ph::_3);
	ecma->input = std::bind(&kernel::target_input, this, ph::_1, ph::_2);
	ecma->ereignis.set(std::bind(&kernel::target_ereignis, this));
	ecma->ereignis.activate();
}


kernel::~kernel()
{
	ecma->ereignis.deactivate();
	ecma->ereignis.set(nullptr);
	ecma->input = nullptr;
	ecma->hookv = nullptr;
	ecma->star = nullptr;
}


void kernel::target_ereignis()
noexcept
{
	slog(LG_DEBUG, "ECMA: Kernel: event");
	const entry_scope entry_scope;
	handle_ereignis();
}


void kernel::handle_ereignis()
try
{

}
catch(exception &e)
{
	slog(LG_ERROR, "!!! KERNEL OOPS !!! %s", e.what());
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "!!! KERNEL PANIC !!! %s", e.what());
	module_abort_all();
}
catch(...)
{
	slog(LG_ERROR, "triple fault");
	throw;
}


void kernel::target_input(sourceinfo_t *const &si,
                          const char *const &input)
noexcept
{
	slog(LG_DEBUG, "ECMA: Kernel: user input [%s]", input);
	const entry_scope entry_scope;
	handle_input(si, input);
}


void kernel::handle_input(sourceinfo_t *const &si,
                          const char *const &input)
try
{
	if(!vorhanden::has_tty(si->su))
	{
		spawn(si, input);
		return;
	}

	function::argv argv
	{
		LS1::string(input)
	};

	auto pid(vorhanden::get_tty(si->su));
	auto task(vorhanden::get(pid));
	enter_user(task, argv);
}
catch(exception &e)
{
	slog(LG_ERROR, "--- !!! KERNEL OOPS !!! --- %s", e.what());
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "--- !!! KERNEL PANIC !!! --- %s", e.what());
	module_abort_all();
}
catch(...)
{
	slog(LG_ERROR, "triple fault");
	throw;
}


void kernel::target_hook(const char *const &name,
                         void *const &data,
                         hook_parser &parser)
noexcept
{
	slog(LG_DEBUG, "ECMA: Kernel: hook: %s", name);
	const entry_scope entry_scope;
	handle_hook(name, data, parser);
}


void kernel::handle_hook(const char *const &name,
                         void *const &data,
                         hook_parser &parser)
try
{


}
catch(exception &e)
{
	slog(LG_ERROR, "!!! KERNEL OOPS !!! %s", e.what());
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "!!! KERNEL PANIC !!! %s", e.what());
	module_abort_all();
}
catch(...)
{
	slog(LG_ERROR, "triple fault");
	throw;
}


void kernel::spawn(sourceinfo_t *const &si,
                   const char *const &input)
{
	auto task(vorhanden::spawn(0, entity(si->smu)->name));
	auto pid(vorhanden::pid(task));
	vorhanden::set_tty(si->su, pid);
	spawn_init(task, input);
}


bool kernel::spawn_init(task &task,
                        const char *const &input)
try
{
	function::prototype proto
	{
		LS1::string("argv")
	};

	function::argv argv
	{
		LS1::array
		{
			LS1::string("hi")
		},
	};

	auto context(vorhanden::ctx(task));
	auto main(function::compile_generator(context, string(input), proto));
	set(task, "main", main);
	enter_user(task, argv);
	return true;
}
catch(exception &e)
{
	terminate(task, e);
	return false;
}



void kernel::enter_user(task &task,
                        function::argv &argv)
try
{
	auto context(vorhanden::ctx(task));
	::call(context, task, task, argv);
}
catch(exception &e)
{
	terminate(task, e);
}


v8::Local<v8::Value>
kernel::target_sys(task &task,
                   v8::Local<v8::Value> &result)
try
{
	using namespace v8;

	const context_scope context_scope(*this);

	slog(LG_DEBUG, "PID[%d] result [%s]\n", vorhanden::pid(task), string(result));

	if(!result)
	{
		terminate(task);
		return {};
	}

	if(result->IsObject())
	{
		auto obj(result.As<Object>());
		handle_result_object(task, obj);
		return {};
	}

	terminate(task, result);
	return {};
}
catch(exception &e)
{
	terminate(task, e);
	return {};
}


void kernel::handle_result_object(task &task,
                                  v8::Local<v8::Object> &result)
{
	if(result->IsGeneratorObject())
	{
		handle_result_object_generator(task, result);
		return;
	}

	if(has(result, "value") && has(result, "done"))
	{
		handle_result_object_iterator(task, result);
		return;
	}
}


void kernel::handle_result_object_iterator(task &task,
                                           v8::Local<v8::Object> &result)
{
	using namespace v8;

	auto value(::get(result, "value"));
	auto done(::get<bool>(result, "done"));
	if(done)
	{
		if(value->IsGeneratorObject())
		{
			auto regenerator(value.As<Object>());
			handle_result_object_generator(task, regenerator);
			return;
		}

		terminate(task, value);
		return;
	}

	if(value->IsUndefined())
		throw error<EINVAL>("Unexpected naked yield.");

	if(!value->IsObject())
		throw error<EINVAL>("Unexpected yield type.");

	auto valobj(value.As<Object>());
	if(has(valobj, priv("fd")))
	{
		handle_descriptor(task, valobj);
		return;
	}

	throw error<EINVAL>("Unexpected non-system yield. (Did you mean to yield* ?)");
}


void kernel::handle_result_object_generator(task &task,
                                            v8::Local<v8::Object> &result)
{
	::set(task, "state", result);

	function::argv a;
	enter_user(task, a);
}


void kernel::handle_descriptor(task &task,
                               v8::Local<v8::Object> &wrapper)
{
	//auto fd(::get<v8::Object>(wrapper, priv("fd")));

	auto &dasein(vorhanden::get(task));
	dasein.status = dasein.SLEEPING;
}


void kernel::terminate(task &task,
                       v8::Local<v8::Value> retval)
try
{
	auto &dasein(vorhanden::get(task));

	slog(LG_DEBUG, "PID[%d] terminated\n", dasein.pid);

	if(!retval)
		retval = v8::Undefined(isolate());

	if(dasein.tty)
	{
		if(!defined(retval))
			dasein.notice("Result undefined. Did you forget the \2return\2 statement?");
		else
			dasein.notice(string(retval));
	}

	vorhanden::remove(dasein.pid);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "--- !!! KERNEL OOPS !!! --- %s", e.what());
	vorhanden::remove(vorhanden::pid(task));
}
catch(...)
{
	throw;
}
