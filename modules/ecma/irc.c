/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct irc
:welt
{
	struct msg
	:function::factory
	{
		call_ret raw_msg(const call_arg &);
		call_ret call(const call_arg &) override;
	}
	msg;

	struct join
	:function::factory
	{
		call_ret call(const call_arg &) override;
	}
	join;
}
static obj;


DECLARE_MODULE_V1
(
	"ecma/irc",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("irc", &obj);
		ecma->add("irc.msg", &obj.msg);
		ecma->add("irc.join", &obj.join);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("irc.join");
		ecma->del("irc.msg");
		ecma->del("irc");
	},
	PACKAGE_STRING,
	"jzk"
);


irc::join::call_ret
irc::join::call(const call_arg &arg)
{
	auto name(::get<0, v8::String>(arg));
	const auto chan(channel_find(string(name)));
	if(!chan)
		throw error<ENOENT>("Target channel not found.");

	::join(chan->name, myservice->me->nick);
	return LS1::boolean(true);
}


irc::msg::call_ret
irc::msg::call(const call_arg &arg)
{
	if(::has(arg, "raw"))
		return raw_msg(arg);

	auto nick(::get<0, v8::String>(arg));
	auto text(::get<1, v8::String>(arg));
	const auto user(user_find(string(nick)));
	if(!user)
		throw error<ENOENT>("Target user not found.");

	const auto from(myservice->me->nick);
	::msg(from, string(nick), "%s", string(text));
	return LS1::boolean(true);
}


irc::msg::call_ret
irc::msg::raw_msg(const call_arg &arg)
{
	auto &dasein(dasein::get(arg));
	const auto mu(dasein.get_owner());
	if(!mu->soper)
		throw error<EACCES>("Not permitted to use the raw version.");

	auto from(::get<0, v8::String>(arg));
	auto nick(::get<1, v8::String>(arg));
	auto text(::get<2, v8::String>(arg));
	::msg(string(from), string(nick), "%s", string(text));
	return LS1::boolean(true);
}
