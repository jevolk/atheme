/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct tty
:welt
{
	std::map<user_t *, pid_t> map;

	set_ret set(const set_arg &, name_arg &, set_val &) override;
	get_ret get(const get_arg &, name_arg &) override;
	del_ret del(const del_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
}
static tty;


DECLARE_MODULE_V1
(
	"ecma/proc_tty",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("proc.tty", &tty);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("proc.tty");
	},
	PACKAGE_STRING,
	"jzk"
);


tty::enu_ret
tty::enu(const enu_arg &arg)
{
	size_t i(0);
	LS1::array ret(map.size());
	std::for_each(begin(map), end(map), [&i, &ret]
	(const auto &pit)
	{
		const auto user(pit.first);
		::set(ret, i++, LS1::string(user->nick));
	});

	return ret;
}


tty::qry_ret
tty::qry(const qry_arg &arg,
         name_arg &name)
{
	auto user(user_find(string(name)));
	if(user && map.count(user))
		return LS1::integer(v8::None);

	return {};
}


tty::del_ret
tty::del(const del_arg &arg,
         name_arg &name)
{
	const auto user(user_find(string(name)));
	if(!user)
		throw error<ENOENT>("User not found");

	return LS1::boolean(map.erase(user));
}


tty::get_ret
tty::get(const get_arg &arg,
         name_arg &name)
{
	const auto user(user_find(string(name)));
	if(!user)
		throw error<ENOENT>("User not found");

	const auto it(map.find(user));
	if(it == end(map))
		throw error<ENOENT>("User has no tty");

	const auto &pid(it->second);
	return LS1::integer(pid);
}


tty::set_ret
tty::set(const set_arg &arg,
         name_arg &name,
         set_val &val)
{
	const auto user(user_find(string(name)));
	if(!user)
		throw error<ENOENT>("User not found");

	const auto pid(as<pid_t>(val, "Must supply a PID integer"));
	map.emplace(user, pid);
	return val;
}
