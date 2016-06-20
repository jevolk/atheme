/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct fd
:welt
{
    set_ret set(const set_arg &, name_arg &, set_val &) override;
    get_ret get(const get_arg &, name_arg &) override;
	del_ret del(const del_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
	call_ret ctor(const call_arg &arg) override;
	void dtor(const call_arg &arg) override;
}
static fd;


struct fds
:welt
{
	uint64_t ctr = 1;
	std::map<uint64_t, v8::Global<v8::Object>> map;

	set_ret iset(const set_arg &, const uint32_t &, set_val &) override;
	get_ret iget(const get_arg &, const uint32_t &) override;
	del_ret idel(const del_arg &, const uint32_t &) override;
	qry_ret iqry(const qry_arg &, const uint32_t &) override;
	enu_ret ienu(const enu_arg &) override;
    call_ret ctor(const call_arg &arg) override;
}
static fds;


DECLARE_MODULE_V1
(
	"ecma/proc_fd",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("proc.fd", &fds);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("proc.fd");
	},
	PACKAGE_STRING,
	"jzk"
);


fds::call_ret
fds::ctor(const call_arg &arg)
{
	using namespace v8;

	auto &dasein(dasein::get(arg));
	auto ret(fd(current(arg)));

	const auto id(ctr++);
	::set(ret, "id", LS1::integer(id));

	auto wrapped(::get<0, Object>(arg));
	::set(ret, "object", wrapped);
	::set(wrapped, priv("fd"), ret);

	auto pid(::get<1, pid_t>(arg));
	::set(ret, "pid", LS1::integer(pid));

	::set(instance(arg), id, ret);
	dasein.fds.emplace(id);
	map.emplace(id, global(ret));
	return ret;
}


fds::enu_ret
fds::ienu(const enu_arg &arg)
{
	size_t i(0);
	LS1::array ret(map.size());
	std::for_each(begin(map), end(map), [&i, &ret]
	(const auto &pit)
	{
		::set(ret, i++, LS1::integer(pit.first));
	});

	return ret;
}


fds::qry_ret
fds::iqry(const qry_arg &arg,
          const uint32_t &idx)
{
	if(map.count(idx))
		return LS1::integer(v8::None);

	return {};
}


fds::del_ret
fds::idel(const del_arg &arg,
          const uint32_t &idx)
try
{
	auto &dasein(dasein::get(arg));
	auto fd(local(map.at(idx)));

	::dtor(fd);
	dasein.fds.erase(idx);
	map.erase(idx);
	return LS1::boolean(true);
}
catch(const std::out_of_range &e)
{
	throw error<ENOENT>("FD does not exist");
}


fds::get_ret
fds::iget(const get_arg &arg,
          const uint32_t &idx)
try
{
	return local(map.at(idx));
}
catch(const std::out_of_range &e)
{
	throw error<ENOENT>("FD does not exist");
}


fds::set_ret
fds::iset(const set_arg &arg,
          const uint32_t &idx,
          set_val &val)
{
	auto &dasein(dasein::get(arg));
	map.emplace(idx, global(as<v8::Object>(val)));
	dasein.fds.emplace(idx);
	return val;
}


void
fd::dtor(const call_arg &arg)
{
	if(!has(arg, "object"))
		return;

	auto object(::get<v8::Object>(arg, "object"));
	::dtor(object);
}


fd::call_ret
fd::ctor(const call_arg &arg)
{
	return instance(arg);
}


fd::enu_ret
fd::enu(const enu_arg &arg)
{
	return {};
}


fd::qry_ret
fd::qry(const qry_arg &arg,
        name_arg &name)
{
	return {};
}


fd::del_ret
fd::del(const del_arg &arg,
        name_arg &name)
{
	return {};
}


fd::get_ret
fd::get(const get_arg &arg,
        name_arg &name)
{
	return {};
}


fd::set_ret
fd::set(const set_arg &arg,
        name_arg &name,
        set_val &val)
{
	switch(hash(string(name)))
	{
		case hash("result"):
		{
			auto pid(::get<pid_t>(arg, "pid"));
			ecma->enter(pid, val);
			return {};
		}

		default:
			return {};
	}
}
