/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"

struct proc
:welt
{
	struct task
	:structure<dasein>
	{
		call_ret call(dasein &dasein, const call_arg &) override;
		dasein *init(const call_arg &) override;
		void dtor(dasein &, const call_arg &) override;

		task();
	}
	task;

	std::map<pid_t, v8::Global<v8::Object>> map;
	pid_t next_pid = 1000;

	set_ret iset(const set_arg &, const uint32_t &, set_val &) override;
	get_ret iget(const get_arg &, const uint32_t &) override;
	del_ret idel(const del_arg &, const uint32_t &) override;
	qry_ret iqry(const qry_arg &, const uint32_t &) override;
	enu_ret ienu(const enu_arg &) override;

	get_ret get(const get_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;

	call_ret ctor(const call_arg &) override;
}
static proc;


DECLARE_MODULE_V1
(
	"ecma/proc",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("proc", &proc);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("proc");
	},
	PACKAGE_STRING,
	"jzk"
);


proc::call_ret
proc::ctor(const call_arg &arg)
{
	auto ret(task(current(arg), function::make_argv(arg)));

	const auto pid(next_pid++);
	::set(ret, "pid", LS1::integer(pid));

	map.emplace(pid, global(ret));
	return ret;
}


proc::enu_ret
proc::enu(const enu_arg &arg)
{
	LS1::array ret
	{{
		LS1::string("self"),
	}};

	return ret;
}


proc::qry_ret
proc::qry(const qry_arg &arg,
          name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("self"):
			return LS1::integer(v8::None);

		default:
			return {};
	}
}


proc::get_ret
proc::get(const get_arg &arg,
          name_arg &name)
{
	switch(hash(string(name)))
	{
		case hash("self"):
		{
			auto pid(dasein::getpid(arg));
			return iget(arg, pid);
		}

		default:
			return welt::get(arg, name);
	}
}


proc::enu_ret
proc::ienu(const enu_arg &arg)
{
	size_t i(0);
	LS1::array ret(map.size());
	std::for_each(begin(map), end(map), [&i, &ret]
	(const auto &pit)
	{
		const auto &pid(pit.first);
		::set(ret, i++, LS1::integer(pid));
	});

	return ret;
}


proc::qry_ret
proc::iqry(const qry_arg &arg,
           const uint32_t &pid)
{
	if(map.count(pid))
		return LS1::integer{v8::None};

	return {};
}


proc::del_ret
proc::idel(const del_arg &arg,
           const uint32_t &idx)
{
	if(idx == 0)
		return LS1::boolean(false);

	if(dasein::getpid(arg) != 0)
		throw error<EACCES>();

	const auto it(map.find(idx));
	if(it == end(map))
		throw error<ENOENT>("Failed to remove non-existent PID");

	auto &task(it->second);
	::dtor(local(task));
	map.erase(it);
	return LS1::boolean(true);
}


proc::get_ret
proc::iget(const get_arg &arg,
           const uint32_t &idx)
{
	const auto it(map.find(idx));
	if(it == end(map))
		throw error<ENOENT>("PID does not exist");

	auto &task(it->second);
	return local(task);
}


proc::set_ret
proc::iset(const set_arg &arg,
           const uint32_t &idx,
           set_val &val)
{
	auto task(v8::Object::New(isolate()));
	auto *const ptr(as<dasein *>(val));
	::set(task, priv("pointer"), LS1::external(ptr));
	map.emplace(idx, global(task));
	return val;
}


proc::task::task()
:structure<dasein>
{{
	{ "pid",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::integer(d.pid);
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			d.pid = as<pid_t>(val);
			return val;
		}
	}},

	{ "ppid",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::integer(d.ppid);
		}
	}},

	{ "time",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::integer(d.cpu_time);
		}}
	},

	{ "runs",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::integer(d.cpu_runs);
		}
	}},

	{ "nice",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::integer(d.cpu_nice);
		}
	}},

	{ "status",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::string({d.status, '\0'});
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			auto s(mustbe<v8::String>(val));
			if(s->Length() != 1)
				throw error<EINVAL>("Must assign a character.");

			const char c(string(s)[0]);
			d.status = static_cast<enum dasein::status>(c);
			return val;
		},
	}},

	{ "tty",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			if(d.tty)
				return LS1::string(d.tty->nick);

			return {};
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			auto nick(mustbe<v8::String>(val));
			const auto user(user_find(string(nick)));
			if(!user)
				throw error<ENOENT>("User not found");

			d.tty = user;
			return val;
		},

		[](dasein &d, const del_arg &arg) -> del_ret
		{
			if(!d.tty)
				return LS1::boolean(false);

			d.tty = nullptr;
			return LS1::boolean(true);
		}
	}},

	{ "user",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return LS1::string(d.get_owner()? entity(d.get_owner())->name : "kernel");
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			auto user(mustbe<v8::String>(val));
			const auto mu(myuser_find(string(user)));
			if(!mu)
				throw error<ENOENT>("Account not found");

			d.set_owner(mu);
			return val;
		},
	}},

	{ "main",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return local(d.erschlossenheit.geworfenheit);
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			using namespace v8;

			if(unlikely(!val))
				return {};

			if(val->IsFunction())
			{
				d.erschlossenheit.geworfenheit = val.As<Function>();
				return val;
			}

			if(val->IsString())
			{
				const context_scope context_scope(d);
				auto src(val.As<String>());
				auto func(function::compile(d.context(), src));
				d.erschlossenheit.geworfenheit = func;
				return val;
			}

			throw error<EINVAL>("Cannot assign this type.");
		},

		[](dasein &d, const del_arg &arg) -> del_ret
		{
			if(!d.erschlossenheit.geworfenheit)
				return LS1::boolean(false);

			d.erschlossenheit.geworfenheit.clear();
			return LS1::boolean(true);
		},

		[](dasein &d, const qry_arg &arg) -> qry_ret
		{
			if(!d.erschlossenheit.geworfenheit)
				return {};

			return LS1::integer(v8::None);
		},
	}},

	{ "global",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return d.global();
		},
	}},

	{ "state",
	{
		[](dasein &d, const get_arg &arg) -> get_ret
		{
			return local(d.erschlossenheit.gelassenheit);
		},

		[](dasein &d, const set_arg &arg, set_val &val) -> set_ret
		{
			d.erschlossenheit.gelassenheit = mustbe<v8::Object>(val);
			return val;
		},

		[](dasein &d, const del_arg &arg) -> del_ret
		{
			if(!d.erschlossenheit.gelassenheit)
				return LS1::boolean(false);

			d.erschlossenheit.gelassenheit.clear();
			return LS1::boolean(true);
		},

		[](dasein &d, const qry_arg &arg) -> qry_ret
		{
			if(!d.erschlossenheit.gelassenheit)
				return {};

			return LS1::integer(v8::None);
		},
	}},
}}
{
}


dasein *
proc::task::init(const call_arg &arg)
{
	auto child(std::make_unique<dasein>(ecma->welt));
	child->ppid = ::get<0, pid_t>(arg);

	if(arg.Length() > 1)
	{
		auto owner(::get<1, const char *>(arg));
		auto mu(myuser_find(owner));
		if(!mu)
			throw error<EINVAL>("Owner account name not found");

		child->set_owner(mu);
	}

	if(child->ppid != -1) try
	{
		auto parent_task(vorhanden::get(child->ppid));
		auto &parent(vorhanden::get(parent_task));

		if(!child->owner)
			child->set_owner(parent.get_owner());

		copy(child->context(), parent.context(), child->global());
	}
	catch(const std::out_of_range &e)
	{
		throw error<EINVAL>("Parent process pid [%d] not found", child->ppid);
	}

	return child.release();
}


void
proc::task::dtor(dasein &dasein,
                 const call_arg &arg)
{
	const std::unique_ptr<struct dasein> _dtor(&dasein);

	{
		const auto &fds(dasein.fds);
		const std::vector<uint64_t> copy(begin(fds), end(fds));
		const context_scope context_scope(dasein);
		for(const auto &id : copy)
			rede::close(id);
	}

	if(dasein.tty)
		vorhanden::del_tty(dasein.tty);
}


proc::task::call_ret
proc::task::call(dasein &dasein,
                 const call_arg &arg)
{
	zuhanden zuhanden;
	auto argv(function::make_argv(arg));
	auto result(zuhanden(dasein, [&dasein, &argv]
	{
		return dasein(argv);
	}));

	auto task(instance(arg));
	return ecma->star(task, result);
}
