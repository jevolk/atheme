/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


function::literal builtin_cd
{"cd", { "where" },
R"(
	if(!etc.env)
		etc.env = Object();

	if(!arguments.length)
	{
		etc.env['PWD'] = this;
		return;
	}

	if(!etc.env['PWD'])
	{
		etc.env['PWD'] = where;
		return;
	}

	etc.env['PWD'] = this[etc.env['PWD'][where]];
)"};


function::literal builtin_ls
{"ls", { "what" },
R"(
	let list = '';
	let path = etc.env? etc.env['PWD'] : null;

	if(!arguments.length && !path)
		what = this;
	else if(!arguments.length && path)
		what = path;
	else if(arguments.lenth && path)
		what = path[what];

	Object.keys(what).forEach(function(key, v)
	{
		list += key + ' ';
	});

	return list;
)"};


function::literal builtin_whoami
{"whoami", {},
R"(
	return proc.self.user;
)"};


function::inlined _time
{
	function::inlined::options{"time"},
	[](const function::inlined::call_arg &arg)
	{
		return LS1::integer(CURRTIME);
	}
};


struct lib
:welt
{
	std::map<myuser_t *, std::map<std::string, v8::Global<v8::Object>>> mydb;

	set_ret set(const set_arg &, name_arg &, set_val &) override;
	get_ret get(const get_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
	call_ret call(const call_arg &) override;
}
static lib;


DECLARE_MODULE_V1
(
	"ecma/lib",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("lib", &lib);
		ecma->add(&builtin_ls);
		ecma->add(&builtin_cd);
		ecma->add(&builtin_whoami);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del(&builtin_whoami);
		ecma->del(&builtin_cd);
		ecma->del(&builtin_ls);
		ecma->del("lib");
	},
	PACKAGE_STRING,
	"jzk"
);


lib::enu_ret
lib::enu(const enu_arg &arg)
{
	auto &builtins(ecma->lib);
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	auto &mudb(mydb[mu]);

	size_t i(0);
	LS1::array ret(mudb.size() + builtins.size());
	std::for_each(begin(mudb), end(mudb), [&i, &ret]
	(const auto &p)
	{
		::set(ret, i++, LS1::string(p.first));
	});

	std::for_each(begin(builtins), end(builtins), [&i, &ret]
	(const auto &p)
	{
		::set(ret, i++, LS1::string(p.first));
	});

	return ret;
}


lib::qry_ret
lib::qry(const qry_arg &arg,
         name_arg &name)
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	const auto it(mydb.find(mu));
	if(it == end(mydb))
		return {};

	auto &mudb(it->second);
	if(!mudb.count(string(name)))
		return {};

	return LS1::integer(v8::None);
}


lib::call_ret
lib::call(const call_arg &arg)
{
	using namespace v8;

	return {};
}


lib::get_ret
lib::get(const get_arg &arg,
         name_arg &name)
{
	using namespace v8;

	{
		auto &builtins(ecma->lib);
		const auto it(builtins.find(string(name)));
		if(it != end(builtins))
		{
			auto &func(*it->second);
			return func(current(arg));
		}

		switch(hash(string(name)))
		{
			case hash("time"):        return _time(current(arg));
		}
	}

	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	const auto it(mydb.find(mu));
	if(it == end(mydb))
		throw error<ENOENT>(string(name));

	auto &mudb(it->second);
	const auto muit(mudb.find(string(name)));
	if(muit == end(mudb))
		throw error<ENOENT>(string(name));

	return local(muit->second);
}


lib::set_ret
lib::set(const set_arg &arg,
         name_arg &name,
         set_val &val)
{
	using namespace v8;

	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu)
		return {};

	auto &mudb(mydb[mu]);
	auto &glob(mudb[string(name)]);

	if(!val->IsObject())
		throw error<EINVAL>("Not an object.");

	glob.Reset(isolate(), val.As<Object>());
	return val;
}
