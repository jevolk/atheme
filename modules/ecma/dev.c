/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct cpu
:object<struct zeug>
{
	struct cr
	:structure<struct zeug>
	{
		cr();
	}
	cr;

	struct sr
	:structure<struct zeug>
	{
		sr();
	}
	sr;

	get_ret get(struct zeug &, const get_arg &, name_arg &) override;
	cpu();
}
static cpu;


struct dev
:welt
{
	struct cpus
	:welt
	{
		get_ret iget(const get_arg &, const uint32_t &) override;
		enu_ret ienu(const enu_arg &) override;
	}
	cpus;
}
static dev;


DECLARE_MODULE_V1
(
	"ecma/dev",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("dev", &dev);
		ecma->add("dev.cpu", &dev.cpus);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("dev.cpu");
		ecma->del("dev");
	},
	PACKAGE_STRING,
	"jzk"
);


dev::cpus::enu_ret
dev::cpus::ienu(const enu_arg &arg)
{
	return LS1::array
	{{
		LS1::integer(0),
	}};
}


dev::cpus::get_ret
dev::cpus::iget(const get_arg &arg,
                const uint32_t &idx)
{
	auto ret(cpu(current(arg)));
	auto &zeug(zeug::zeug::get());
	::set(ret, priv("pointer"), LS1::external(&zeug));
	return ret;
}



cpu::cpu()
{
	child_add("cr", &cr);
	child_add("sr", &sr);
}


cpu::get_ret
cpu::get(struct zeug &zeug,
         const get_arg &arg,
         name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("cr"):
		{
			auto ret(cr(current(arg)));
			::set(ret, priv("pointer"), ::get(arg, priv("pointer")));
			return ret;
		}

		case hash("sr"):
		{
			auto ret(sr(current(arg)));
			::set(ret, priv("pointer"), ::get(arg, priv("pointer")));
			return ret;
		}

		default:
			return object::get(zeug, arg, name);
	}
}


cpu::cr::cr()
:structure<struct zeug>
{{
	{ "preempt",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(true);
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}
	}},

	{ "sigmask",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return {};
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}}
	},

	{ "memadj",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return {};
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}
	}},

	{ "mempress",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return {};
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}
	}},

	{ "memlow",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return {};
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}
	}},

	{ "eventlog",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zeug.opts.event_logging);
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			ecma->zeug.opts.event_logging = mustbe<v8::Boolean>(val)->Value();
			return val;
		}
	}},

	{ "mtpolicy",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return {};
		},

		[](struct zeug &zeug, const set_arg &arg, set_val &val) -> set_ret
		{
			return {};
		}
	}},
}}
{
}


cpu::sr::sr()
:structure<struct zeug>
{{
	{ "time",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::integer(ecma->zeug.stats.time);
		}
	}},

	{ "runs",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::integer(ecma->zeug.stats.runs);
		}}
	},

	{ "busy",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zeug.in_context());
		}
	}},

	{ "dead",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zeug.is_dead());
		}
	}},

	{ "locked",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zeug.is_locked());
		}
	}},

	{ "terminating",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zeug.is_terminating());
		}
	}},
/*
	{ "presig",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zuhanden.preempt_signaled);
		}
	}},

	{ "prereq",
	{
		[](struct zeug &zeug, const get_arg &arg) -> get_ret
		{
			return LS1::boolean(ecma->zuhanden.preempt_requested);
		}
	}},
*/
}}
{
}
