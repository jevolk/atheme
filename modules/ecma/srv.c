/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct command
:object<command_t>
{
	call_ret call(command_t &, const call_arg &) override;
}
static command;


struct service
:object<service_t>
{
	get_ret get(service_t &, const get_arg &, name_arg &) override;
	enu_ret enu(service_t &, const enu_arg &) override;
}
static service;


struct srv
:welt
{
	struct hook
	:welt
	{
		del_ret del(const del_arg &, name_arg &) override;
		call_ret call(const call_arg &) override;
	}
	hook;

	get_ret get(const get_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
}
static srv;


DECLARE_MODULE_V1
(
	"ecma/srv",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("srv", &srv);
		ecma->add("srv.hook", &srv.hook);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("srv.hook");
		ecma->del("srv");
	},
	PACKAGE_STRING,
	"jzk"
);


srv::enu_ret
srv::enu(const enu_arg &arg)
{
	size_t i(0);
	LS1::array ret(mowgli_patricia_size(services_nick));
	for_each<service_t>(services_nick, [&i, &ret](auto &service)
	{
		::set(ret, i++, LS1::string(service->internal_name));
	});

	return ret;
}


srv::get_ret
srv::get(const get_arg &arg,
         name_arg &name)
{
	auto s(service_find(string(name)));
	return s? service(s) : throw error<ENOENT>("Not a service.");
}


service::enu_ret
service::enu(service_t &s,
             const enu_arg &arg)
{
	size_t i(0);
	LS1::array ret(mowgli_patricia_size(s.commands));
	for_each<command_t>(s.commands, [&i, &ret](auto &c)
	{
		::set(ret, i++, LS1::string(c->name));
	});

	return ret;
}


service::get_ret
service::get(service_t &s,
             const get_arg &arg,
             name_arg &name)
{
	auto c(command_find(s.commands, string(name)));
	auto ret(c? command(c) : throw error<ENOENT>("Bad command or file name."));
	::set(ret, priv("service"), LS1::external(&s));
	return ret;
}



uint haxer;
char hax[32][BUFSIZE];

command::call_ret
command::call(command_t &c,
              const call_arg &arg)
{
	auto s(::get<service_t *>(arg, priv("service")));

	const size_t parc(arg.Length());
	std::array<char *, 16> parv {0};
	for(size_t i(0); i < parc && i < parv.size() - 1; ++i)
		parv[i] = (char *)string(arg[i]);

	sourceinfo_t si {0};
	sourceinfo_vtable siv {0};
	si.v = &siv;

	siv.cmd_success_nodata = []
	(sourceinfo_t *const si, const char *const message)
	{
		mowgli_strlcpy(hax[haxer], message, sizeof(hax[haxer]));
		haxer = (haxer + 1) % 32;
	};

	command_exec(s, &si, &c, parc, parv.data());

	LS1::array ret(haxer);
	for(size_t i(0); i < haxer; i++)
		::set(ret, i, LS1::string(hax[i]));

	haxer = 0;
	return ret;
}



srv::hook::call_ret
srv::hook::call(const call_arg &arg)
{
	auto &dasein(dasein::get(arg));

	if(arg.Length() < 2)
		throw error<EINVAL>("Usage: (hook_name, function)");

	auto name(arg[0]);
	if(!name->IsString())
		throw error<EINVAL>("First argument must be a string naming the hook.");

	auto func(arg[1]);
	if(!func->IsFunction())
		throw error<EINVAL>("Second argument must be a function callback.");

	return {};
}


srv::hook::del_ret
srv::hook::del(const del_arg &arg,
               name_arg &name)
{
	printf("Deleting %s\n", string(name));

	return {};
}
