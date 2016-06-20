/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct bin
:welt
{
	std::map<myuser_t *, std::map<std::string, v8::Global<v8::UnboundScript>>> mydb;

	set_ret set(const set_arg &, name_arg &, set_val &) override;
	get_ret get(const get_arg &, name_arg &) override;
	del_ret del(const del_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
	call_ret call(const call_arg &arg) override;
}
static _bin;


DECLARE_MODULE_V1
(
	"ecma/bin",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("bin", &_bin);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("bin");
	},
	PACKAGE_STRING,
	"jzk"
);


bin::call_ret
bin::call(const call_arg &arg)
try
{
	const auto name(arg[0]);
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	auto compiled(local(mydb.at(mu).at(string(name))));
	auto bound(compiled->BindToCurrentContext());
	return maybe<js_error>([&bound, &arg]
	{
		return bound->Run(current(arg));
	});
}
catch(const std::out_of_range &e)
{
	throw error<ENOENT>(string(arg[0]));
}


bin::enu_ret
bin::enu(const enu_arg &arg)
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	size_t i(0);
	auto &mudb(mydb[mu]);
	LS1::array ret(mudb.size());
	std::for_each(begin(mudb), end(mudb), [&i, &ret]
	(const auto &p)
	{
		::set(ret, i++, LS1::string(p.first));
	});

	return ret;
}


bin::qry_ret
bin::qry(const qry_arg &arg,
         name_arg &name)
try
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	const auto it(mydb.find(mu));
	if(it == end(mydb))
		return {};

	auto &mudb(it->second);
	if(!mudb.count(string(name)))
		return {};

	return LS1::integer(v8::None);
}
catch(const std::out_of_range &e)
{
	throw error<ENOENT>();
}


bin::del_ret
bin::del(const del_arg &arg,
         name_arg &name)
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	const auto it(mydb.find(mu));
	if(it == end(mydb))
		return LS1::boolean(false);

	auto &mudb(it->second);
	return LS1::boolean(mudb.erase(string(name)));
}


bin::get_ret
bin::get(const get_arg &arg,
         name_arg &name)
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	const auto it(mydb.find(mu));
	if(it == end(mydb))
		throw error<ENOENT>(string(name));

	auto &mudb(it->second);
	if(!mudb.count(string(name)))
		throw error<ENOENT>(string(name));

	const auto fork_exec
	{
		" sys.exec_argv = [ '%s', di, si, dx, cx, r8, r9 ];  "
		" return sys.fork();                                 "
	};

	char script[256];
	snprintf(script, sizeof(script), fork_exec, string(name));
	function::literal wrapper
	{
		"fork_exec",
		{ "di", "si", "dx", "cx", "r8", "r9" },
		script
	};

	return wrapper(current(arg));
}


bin::set_ret
bin::set(const set_arg &arg,
         name_arg &name,
         set_val &val)
{
	using namespace v8;

	auto &dasein(dasein::get(arg));
	auto mu(dasein.get_owner());
	if(!mu)
		return {};

	auto code(maybe<type_error>(val->ToString(current(arg))));
	auto &mudb(mydb[mu]);

	ScriptCompiler::Source source(code);
	auto compiled(maybe<syntax_error>([&source]
	{
		return ScriptCompiler::CompileUnboundScript(isolate(), &source);
	}));

	mudb.emplace(string(name), global(compiled));
	return val;
}
