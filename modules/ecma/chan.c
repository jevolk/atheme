/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct chan
:welt
{
	template<class T> static LS1::string get_name(const T &arg);
	template<class T> static channel_t *channel_find(const T &arg);
	template<class T> static mychan_t *mychan_find(const T &arg);

	struct users
	:welt
	{
		qry_ret qry(const qry_arg &arg, name_arg &) override;
		enu_ret enu(const enu_arg &arg) override;
	}
	users;

	get_ret get_users(const get_arg &arg);

	get_ret get_topic(const get_arg &arg);
	set_ret set_topic(const set_arg &arg, set_val &);

	LS1::string get_mode(const get_arg &arg);
	LS1::string get_key(const get_arg &arg);

	set_ret set(const set_arg &arg, name_arg &, set_val &) override;
	get_ret get(const get_arg &arg, name_arg &) override;
	qry_ret qry(const qry_arg &arg, name_arg &) override;
	enu_ret enu(const enu_arg &arg) override;
}
static _chan;


struct chans
:welt
{
	call_ret call(const call_arg &arg) override;
	qry_ret qry(const qry_arg &arg, name_arg &) override;
	enu_ret enu(const enu_arg &arg) override;
}
static _chans;


DECLARE_MODULE_V1
(
	"ecma/chan",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("chan", &_chans);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("chan");
	},
	PACKAGE_STRING,
	"jzk"
);



chans::enu_ret
chans::enu(const enu_arg &arg)
{
	const size_t num(mowgli_patricia_size(chanlist));
	LS1::array ret(num);

	size_t i(0);
	void *cp;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(cp, &state, chanlist)
	{
		const auto c(reinterpret_cast<channel_t *>(cp));
		::set(ret, i, LS1::string(c->name));
		++i;
	}

	return ret;
}


chans::qry_ret
chans::qry(const qry_arg &arg,
           name_arg &name)
{
	return ::channel_find(string(name))? LS1::integer{v8::None}:
	                                     LS1::integer{v8::DontEnum};
}


chans::call_ret
chans::call(const call_arg &arg)
{
	channel_t *const c(::channel_find(string(arg[0])));
	if(!c)
		throw error<ENOENT>("Channel not found.");

	auto ret(_chan(current(arg)));
	::set(ret, priv("name"), arg[0]);
	return ret;
}


chan::enu_ret
chan::enu(const enu_arg &arg)
{
	return LS1::array
	{{
		LS1::string("key"),
		LS1::string("mode"),
		LS1::string("topic"),
		LS1::string("users"),
	}};
}


chan::qry_ret
chan::qry(const qry_arg &arg,
          name_arg &name)
{
	switch(hash(string(name)))
	{
		default:
			return LS1::integer{v8::None};
	}
}


chan::set_ret
chan::set(const set_arg &arg,
          name_arg &name,
          set_val &val)
{
	switch(hash(string(name)))
	{
		case hash("topic"):        return set_topic(arg, val);
		default:                   return {};
	}
}


chan::set_ret
chan::set_topic(const set_arg &arg,
                set_val &val)
{
	const auto c(channel_find(arg));
	if(!c)
		return {};

	if(!validtopic(string(val)))
		throw error<EINVAL>("Invalid topic.");

	auto ts(c->topicts);
	auto setter(myservice->me->nick);
	handle_topic(c, setter, CURRTIME, string(val));
	topic_sts(c, myservice->me, setter, CURRTIME, ts, string(val));
	return val;
}


chan::get_ret
chan::get(const get_arg &arg,
          name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("key"):          return get_key(arg);
		case hash("mode"):         return get_mode(arg);
		case hash("topic"):        return get_topic(arg);
		case hash("users"):        return get_users(arg);
		default:                   return {};
	}
}


chan::get_ret
chan::get_users(const get_arg &arg)
{
	auto ret(users(current(arg)));
	auto name(::get(instance(arg), priv("name")));
	::set(ret, priv("name"), name);
	return ret;
}


chan::get_ret
chan::get_topic(const get_arg &arg)
{
	const auto c(channel_find(arg));
	auto ret(v8::Object::New(isolate()));
	::set(ret, LS1::string("text"), LS1::string(c->topic));
	::set(ret, LS1::string("setter"), LS1::string(c->topic_setter));
	::set(ret, LS1::string("ts"), LS1::integer(c->topicts));
	return ret;
}


LS1::string chan::get_key(const get_arg &arg)
{
	return channel_find(arg)->key;
}



LS1::string chan::get_mode(const get_arg &arg)
{
	return channel_modes(channel_find(arg), true);
}


template<class T>
mychan_t *chan::mychan_find(const T &arg)
{
	const auto ret(::mychan_find(string(get_name(arg))));
	return ret?: throw error<ENOENT>("Channel not registered.");
}


template<class T>
channel_t *chan::channel_find(const T &arg)
{
	const auto ret(::channel_find(string(get_name(arg))));
	return ret?: throw error<ENOENT>("Channel not found.");
}


template<class T>
LS1::string chan::get_name(const T &arg)
{
	return { ::get(instance(arg), priv("name")) };
}


chan::users::enu_ret
chan::users::enu(const enu_arg &arg)
{
	const auto channel(chan::channel_find(arg));
	if(!channel)
		return {};

	size_t i(0);
	mowgli_node_t *n, *tn;
	LS1::array ret(channel->nummembers);
	MOWGLI_ITER_FOREACH_SAFE(n, tn, channel->members.head)
	{
		const auto cu(reinterpret_cast<chanuser_t *>(n->data));
		ret->Set(i++, LS1::string(cu->user->nick));
	}

	return ret;
}


chan::users::qry_ret
chan::users::qry(const qry_arg &arg,
                 name_arg &name)
{
	const auto channel(chan::channel_find(arg));
	const auto u(user_find(string(name)));
	if(!u)
		return {};

	const auto cu(chanuser_find(channel, u));
	if(!cu)
		return {};

	return LS1::integer{v8::None};
}
