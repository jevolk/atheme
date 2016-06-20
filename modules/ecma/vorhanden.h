/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

namespace vorhanden
{
	using namespace v8;
	using task = Local<Object>;

	auto proc()                                  { return ::get<Object>("proc");                   }
	auto proc_tty()                              { return ::get<Object>(proc(), "tty");            }
	auto nick(const user_t *const &user)         { return LS1::string(user->nick);                 }

	task get(const pid_t &pid)                   { return ::get<Object>(proc(), pid);              }
	auto &get(const task &task)                  { return *::get<dasein *>(task, priv("pointer")); }
	auto pid(const task &task)                   { return ::get<pid_t>(task, "pid");               }
	task global(const task &task)                { return ::get<Object>(task, "global");           }
	task global(const pid_t &pid)                { return global(get(pid));                        }
	auto ctx(const task &task)                   { return get(task).context();                     }
	auto ctx(const pid_t &pid)                   { return ctx(get(pid));                           }

	bool has_tty(const user_t *const &user)      { return has(proc_tty(), nick(user));             }
	pid_t get_tty(const user_t *const &user)     { return ::get<pid_t>(proc_tty(), nick(user));    }
	bool del_tty(const user_t *const &);
	void set_tty(const user_t *const &, const pid_t & = 0);

	bool remove(const pid_t &pid)                { return ::del(proc(), pid);                      }
	task spawn(const pid_t &ppid = 0, const char *const &owner = nullptr);
}


inline
vorhanden::task
vorhanden::spawn(const pid_t &ppid,
                 const char *const &owner)
{
	return owner? ctorgv("proc", LS1::integer(ppid), LS1::string(owner)):
	              ctorgv("proc", LS1::integer(ppid));
}


inline
void vorhanden::set_tty(const user_t *const &user,
                        const pid_t &pid)
{
	using namespace v8;

	if(has(proc_tty(), nick(user)))
		del_tty(user);

	::set(get(pid), "tty", nick(user));
	::set(proc_tty(), nick(user), LS1::integer(pid));
}


inline
bool vorhanden::del_tty(const user_t *const &user)
{
	if(!has_tty(user))
		return false;

	auto pid(::get<pid_t>(proc_tty(), nick(user)));
	auto task(get(pid));
	if(::has(task, "tty"))
		::del(task, "tty");

	::del(proc_tty(), nick(user));
	return true;
}
