/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct usr
:welt
{
	template<class T> static LS1::string get_name(const T &arg);
	template<class T> static myuser_t *myuser_find(const T &arg);
	template<class T> static user_t *user_find(const T &arg);

	LS1::string get_email(const get_arg &arg);
	LS1::integer get_registered(const get_arg &arg);
	LS1::integer get_lastlogin(const get_arg &arg);
	get_ret get_nicks(const get_arg &arg);

	set_ret set(const set_arg &, name_arg &, set_val &) override;
	get_ret get(const get_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
}
static _usr;


struct usrs
:welt
{
	call_ret call(const call_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;
}
static _usrs;


DECLARE_MODULE_V1
(
	"ecma/usr",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("usr", &_usrs);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("usr");
	},
	PACKAGE_STRING,
	"jzk"
);



usrs::enu_ret
usrs::enu(const enu_arg &arg)
{
	size_t num(0);
	myentity_foreach_t(ENT_USER, []
	(myentity_t *const ent, void *const num)
	{
		++(*reinterpret_cast<size_t *>(num));
		return 0;
	},
	&num);

	struct priv
	{
		LS1::array ret;
		size_t i;
	} s { { num }, 0 };
	myentity_foreach_t(ENT_USER, []
	(myentity_t *const ent, void *const priv)
	{
		auto &s(*reinterpret_cast<struct priv *>(priv));
		::set(s.ret, s.i, LS1::string(ent->name));
		s.i++;
		return 0;
	},
	&s);

	return s.ret;
}


usrs::qry_ret
usrs::qry(const qry_arg &arg,
          name_arg &name)
{
	return ::myuser_find(string(name))? LS1::integer{v8::None}:
	                                    LS1::integer{v8::DontEnum};
}


usrs::call_ret
usrs::call(const call_arg &arg)
{
	auto *const mu(::myuser_find(string(arg[0])));
	if(!mu)
		throw error<ENOENT>("Account not found.");

	auto ret(_usr(current(arg)));
	::set(ret, priv("name"), arg[0]);
	return ret;
}



usr::enu_ret
usr::enu(const enu_arg &arg)
{
	return LS1::array
	{{
		LS1::string("email"),
		LS1::string("registered"),
		LS1::string("lastlogin"),
		LS1::string("nicks"),
	}};
}


usr::qry_ret
usr::qry(const qry_arg &arg,
         name_arg &name)
{
	switch(hash(string(name)))
	{
		default:
			return LS1::integer{v8::None};
	}
}


usr::set_ret
usr::set(const set_arg &arg,
         name_arg &name,
         set_val &val)
{
	switch(hash(string(name)))
	{
		default:                   return {};
	}
}


usr::get_ret
usr::get(const get_arg &arg,
         name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("email"):        return get_email(arg);
		case hash("registered"):   return get_registered(arg);
		case hash("lastlogin"):    return get_lastlogin(arg);
		case hash("nicks"):        return get_nicks(arg);
		default:                   return {};
	}
}


usr::get_ret usr::get_nicks(const get_arg &arg)
{
	using namespace v8;

	const auto mn_object([&arg]
	(const mynick_t *const mn)
	{
		auto ret(Object::New(isolate()));
		::set(ret, LS1::string("registered"), LS1::integer(mn->registered));
		::set(ret, LS1::string("lastseen"), LS1::integer(mn->lastseen));
		return ret;
	});

	const auto mu(myuser_find(arg));
	auto ret(Map::New(isolate()));
	if(!mu->nicks.count)
	{
		mynick_t mn;
		mn.lastseen = mu->lastlogin;
		mn.registered = mu->registered;
		::set(ret, LS1::string(entity(mu)->name), mn_object(&mn));
		return ret;
	}

	mowgli_node_t *n, *tn;
	MOWGLI_ITER_FOREACH_SAFE(n, tn, mu->nicks.head)
	{
		const auto mn(reinterpret_cast<mynick_t *>(n->data));
		::set(ret, LS1::string(mn->nick), mn_object(mn));
	}

	return ret;
}


LS1::integer usr::get_lastlogin(const get_arg &arg)
{
	const auto mu(myuser_find(arg));
	return LS1::integer(mu->lastlogin);
}


LS1::integer usr::get_registered(const get_arg &arg)
{
	const auto mu(myuser_find(arg));
	return LS1::integer(mu->registered);
}


LS1::string usr::get_email(const get_arg &arg)
{
	const auto mu(myuser_find(arg));
	return LS1::string(mu->email);
}


template<class T>
myuser_t *usr::myuser_find(const T &arg)
{
	const auto ret(::myuser_find(string(get_name(arg))));
	return ret?: throw error<ENOENT>("User not registered.");
}


template<class T>
user_t *usr::user_find(const T &arg)
{
	const auto ret(::user_find(string(get_name(arg))));
	return ret?: throw error<ENOENT>("User not found.");
}


template<class T>
LS1::string usr::get_name(const T &arg)
{
	return { ::get(instance(arg), priv("name")) };
}
