/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct dasein
{
	enum status : char
	{
		RUNNING      = 'R',       // Currently being executed.
		ERROR        = 'E',       // Real host-level error has occurred.
		PREEMPT      = 'P',       // Dirty interruption by the scheduler.
		HALT         = 'H',       // Clean termination with undefined result.
		EXCEPT       = 'e',       // Clean abort (nhandled js exception is result).
		SLEEPING     = 'S',       // Clean point waiting for events
		STOPPED      = 's',       // SIGSTOP suspension
	};

	pid_t ppid;                                  // Parent process ID
	pid_t pid;                                   // Process ID
	const struct policy *policy;                 // Process policy attributes
	struct lichtung lichtung;                    // Data tape
	struct erschlossenheit erschlossenheit;      // Instruction tape
	int64_t cpu_time;                            // Time consumed
	uint64_t cpu_runs;                           // Number of times executed
	int8_t cpu_nice;                             // Task priority
	status status;                               // Current status character
	myuser_t *owner;                             // Owner of process
	user_t *tty;                                 // When it disconnects it becomes NULL and also a SIGHUP.
	std::set<uint64_t> fds;                      // Open descriptors, values are global

	auto context() const                         { return local(lichtung.context);                 }
	auto context()                               { return local(lichtung.context);                 }
	auto global() const                          { return context()->Global();                     }
	auto global()                                { return context()->Global();                     }

	operator v8::Local<v8::Context>() const      { return context();                               }
	operator v8::Local<v8::Context>()            { return context();                               }
	operator v8::Local<v8::Object>() const       { return global();                                }
	operator v8::Local<v8::Object>()             { return global();                                }

	auto context_size() const                    { return context()->EstimatedSize();              }
	auto time_remaining() const                  { return policy->cpu_quota - cpu_time;            }
	auto get_owner() const                       { return owner;                                   }
	void set_owner(auto owner)                   { this->owner = owner;                            }

	bool notice(const char *fmt, ...) PRINTFLIKE(2, 3);

	v8::MaybeLocal<v8::Value> operator()(function::argv &);
	v8::MaybeLocal<v8::Value> operator()(const function::argv & = {});

	dasein(welt &, v8::Local<v8::Object> global = {});
	dasein(dasein &&) = delete;
	dasein(const dasein &) = delete;
	virtual ~dasein() noexcept;

	// Extract an instance from upstream types:
	static dasein &get(const v8::Local<v8::Context>);
	template<class T> static dasein &get(const v8::PropertyCallbackInfo<T> &);
	template<class T> static dasein &get(const v8::FunctionCallbackInfo<T> &);
	template<class T> static void closure(T&& contextual, const std::function<void (dasein &)> &);
	template<class T> static pid_t getpid(T&& contextual);
};


inline
dasein::dasein(welt &welt,
               v8::Local<v8::Object> global)
:ppid{0}
,pid{0}
,policy{&policy_default}
,lichtung{welt, global}
,erschlossenheit{}
,cpu_time{0}
,cpu_runs{0}
,cpu_nice{policy->cpu_nice}
,status{HALT}
,owner{nullptr}
,tty{nullptr}
{
	lichtung.set(lichtung.DASEIN, this);
}


inline
dasein::~dasein()
noexcept
{
}


v8::MaybeLocal<v8::Value>
dasein::operator()(const function::argv &a)
{
	auto b(a);
	return operator()(b);
}


v8::MaybeLocal<v8::Value>
dasein::operator()(function::argv &a)
{
	return erschlossenheit(global(), a);
}


inline
bool dasein::notice(const char *const fmt,
                    ...)
{
	if(!tty)
		return false;

	va_list ap;
	va_start(ap, fmt);
	char buf[BUFSIZE];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	const char *const from(myservice->me->nick);
	const char *const to(tty->nick);
	::notice(from, to, "[\2%d\2]: %s", pid, buf);
	va_end(ap);
	return true;
}


template<class T>
pid_t dasein::getpid(T&& contextual)
{
	pid_t ret;
	closure(contextual, [&ret]
	(dasein &dasein)
	{
		ret = dasein.pid;
	});

	return ret;
}


template<class T>
void dasein::closure(T&& contextual,
                     const std::function<void (dasein &)> &lambda)
{
	lambda(get(contextual));
}


template<class T>
dasein &dasein::get(const v8::FunctionCallbackInfo<T> &arg)
{
	auto &lich(lichtung::get(arg));
	return *static_cast<dasein *>(lich.get_ptr(lichtung::field::DASEIN));
}


template<class T>
dasein &dasein::get(const v8::PropertyCallbackInfo<T> &arg)
{
	auto &lich(lichtung::get(arg));
	return *static_cast<dasein *>(lich.get_ptr(lichtung::field::DASEIN));
}


inline
dasein &dasein::get(const v8::Local<v8::Context> context)
{
	auto &lich(lichtung::get(context));
	return *static_cast<dasein *>(lich.get_ptr(lichtung::field::DASEIN));
}
