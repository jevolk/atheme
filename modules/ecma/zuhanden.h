/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct zuhanden
{
	enum { OURS, THEIRS };
	using closure = std::function<v8::MaybeLocal<v8::Value> ()>;

	struct zeug &zeug;                           // Engine found through thread-local reference
	struct zuhanden *theirs;                     // Allows recursive execution by stacking active instance
	struct sigaction sa_pre[2];                  // Handler for signaling preemption
	struct itimerval tv_pre[2];                  // Timer for signaling preemption
	bool preempted;                              // Indicates preemption requested (signaled or otherwise)
	int signaled;                                // Indicates signal handler was entered with last signum

	static void signal_handler(int) noexcept;
	void timer_uninstall();
	void timer_install();
	void sig_uninstall();
	void sig_install();
	void interrupt();
	void resume();

	v8::MaybeLocal<v8::Value> run(dasein &, const closure &);

  public:
	v8::Local<v8::Value> operator()(dasein &, const closure &);

	zuhanden();
	~zuhanden();
}

static thread_local *zuhanden_active;


inline
zuhanden::zuhanden()
:zeug(zeug::zeug::get())
,theirs{zuhanden_active}
,sa_pre
{
	[this]
	{
		struct sigaction ours {0};
		ours.sa_handler = signal_handler;
		ours.sa_mask = zeug.opts.sigmask;
		return ours;
	}()
}
,tv_pre{0}
,preempted{false}
,signaled{0}
{
}


inline
zuhanden::~zuhanden()
noexcept
{
}


inline
v8::Local<v8::Value>
zuhanden::operator()(dasein &dasein,
                     const closure &lambda)
{
	using namespace v8;

	const auto &policy(*dasein.policy);
	const auto &interval(policy.cpu_interval);
	const auto remaining(std::min(dasein.time_remaining(), interval));

	auto &it_value(tv_pre[OURS].it_value);
	auto &it_interval(tv_pre[OURS].it_interval);
	it_value = usec_to_tv(remaining);

	const context_scope context_scope(dasein);
	trycatch_scope tc;

	dasein.status = dasein.RUNNING;
	auto result(run(dasein, lambda));
	dasein.status = preempted?        dasein.PREEMPT:
	                tc->HasCaught()?  dasein.EXCEPT:
	                                  dasein.HALT;

	dasein.cpu_time += remaining - tv_to_usec(it_value);
	if(tc->HasTerminated() || !tc->CanContinue())             // Reset engine after forceful termination.
		resume();

	switch(dasein.status)
	{
		case dasein.PREEMPT:
			return string("killed: CPU slice exceeded.");

		case dasein.EXCEPT:
			throw exception(tc->Message());

		default:
		case dasein.HALT:
			return test(result)? checked(result):
			                     Local<Value>(Undefined(isolate()));
	}
}

inline
v8::MaybeLocal<v8::Value>
zuhanden::run(dasein &dasein,
              const closure &lambda)
try
{
	const scope s([this, &dasein]
	{
		timer_uninstall();
		zuhanden_active = theirs;
		sig_uninstall();
	});

	// Allow multiple use of zuhanden instance
	preempted = false;
	signaled = 0;

	++dasein.cpu_runs;
	zuhanden_active = this;   // TODO: ABA  -|
	sig_install();            // TODO: ABA  -|
	timer_install();
	return lambda();
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Core execution error: %s", e.what());
	dasein.status = dasein.ERROR;
	return string(e.what());
}


inline
void zuhanden::resume()
{
	zeug.resume();
}


inline
void zuhanden::interrupt()
{
	preempted = true;
	zeug.terminate();
}


inline
void zuhanden::sig_install()
{
	syscall(::sigaction, SIGVTALRM, &sa_pre[OURS], &sa_pre[THEIRS]);
}


inline
void zuhanden::sig_uninstall()
{
	syscall(::sigaction, SIGVTALRM, &sa_pre[THEIRS], nullptr);
}


inline
void zuhanden::timer_install()
{
	syscall(::setitimer, ITIMER_VIRTUAL, &tv_pre[OURS], &tv_pre[THEIRS]);
}


inline
void zuhanden::timer_uninstall()
{
	syscall(::setitimer, ITIMER_VIRTUAL, &tv_pre[THEIRS], &tv_pre[OURS]);
}


inline
void zuhanden::signal_handler(int signum)
noexcept
{
	assert(zuhanden_active);
	auto &zuhanden(*zuhanden_active);
	zuhanden.signaled = signum;
	{
		const isolate_scope isolate_scope;
		zuhanden.interrupt();
	}
	std::atomic_signal_fence(std::memory_order_release);
}
