/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct sys
:welt
{
	struct log
	:function::factory
	{
		static int reflect(const v8::Local<v8::Value> &str);

		call_ret call(const call_arg &arg) override;
	}
	log;

	struct fork
	:welt
	{
		call_ret call(const call_arg &arg) override;
	}
	fork;

	struct uname
	:function::factory
	{
		call_ret call(const call_arg &arg) override;
	}
	uname;

	struct getpid
	:function::factory
	{
		call_ret call(const call_arg &arg) override;
	}
	getpid;

	get_ret get(const get_arg &, name_arg &) override;
}
static sys;


DECLARE_MODULE_V1
(
	"ecma/sys",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("sys", &sys);
		ecma->add("sys.log", &sys.log);
		ecma->add("sys.fork", &sys.fork);
		ecma->add("sys.uname", &sys.uname);
		ecma->add("sys.getpid", &sys.getpid);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("sys.getpid");
		ecma->del("sys.uname");
		ecma->del("sys.fork");
		ecma->del("sys.log");
		ecma->del("sys");
	},
	PACKAGE_STRING,
	"jzk"
);


inline
sys::get_ret
sys::get(const get_arg &arg,
         name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("stdin"):      return LS1::string_object("stdin");
		case hash("stdout"):     return LS1::string_object("stdout");
		case hash("stderr"):     return LS1::string_object("stderr");
		default:                 return welt::get(arg, name);
	}
}


sys::log::call_ret
sys::log::call(const call_arg &arg)
{
	auto &dasein(dasein::get(arg));

	if(arg.Length() == 0)
	{
		slog(LG_DEBUG, "--- mark ---");
		return {};
	}

	const auto a0(arg[0]);
	if(arg.Length() == 1)
	{
		string(a0, [](const char *const &msg)
		{
			slog(LG_DEBUG, msg);
		});

		return {};
	}

	if(unlikely(!a0->IsString()))
		throw type_error("Facility must be a string");

	const int facility(reflect(a0));
	string(arg[1], [&facility](const char *const &msg)
	{
		slog(facility, msg);
	});

	return {};
}


int sys::log::reflect(const v8::Local<v8::Value> &str)
{
	if(!str || !str->IsString())
		throw type_error("Log facility must be a string");

	switch(hash(string(str)))
	{
		case hash("NONE"):       return LG_NONE;
		case hash("INFO"):       return LG_INFO;
		case hash("ERROR"):      return LG_ERROR;
		case hash("IOERROR"):    return LG_IOERROR;
		case hash("DEBUG"):      return LG_DEBUG;
		case hash("VERBOSE"):    return LG_VERBOSE;
		default:                 throw error<EINVAL>("Invalid log facility");
	}
}


sys::getpid::call_ret
sys::getpid::call(const call_arg &arg)
{
	auto &dasein(dasein::get(arg));
	return LS1::integer(dasein.pid);
}


sys::uname::call_ret
sys::uname::call(const call_arg &arg)
{
	return {};
}


sys::fork::call_ret
sys::fork::call(const call_arg &arg)
{
	return {};
}
