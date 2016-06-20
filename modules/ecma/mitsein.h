/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct mitsein
{
	enum { IGNORE, OURS, THEIRS, ORIGINAL };
	using closure = std::function<bool (dasein &)>;

	struct zeug &zeug;
	struct ereignis &ereignis;
	struct sigaction sa_sigio[4];
	bool sigio_ready, sigio_caught;
	std::deque<dasein *, allocator<dasein *>> queue;

	static void signal_handler(int) noexcept;
	static bool uplink_ready();
	void sigio_uninstall();
	void sigio_install();
	bool sigio_disable();
	bool sigio_enable();

	void sort_queue();

  public:
	void operator()(const closure &);
	bool cancel(dasein &);
	bool add(dasein &);

	mitsein(struct zeug &, struct ereignis &);
	~mitsein();
}

static thread_local *mitsein_instance;


inline
mitsein::mitsein(struct zeug &zeug,
                 struct ereignis &ereignis)
:zeug{zeug}
,ereignis{ereignis}
,sa_sigio
{
	[]{
		struct sigaction ignore;
		ignore.sa_handler = SIG_IGN;
		return ignore;
	}(),
	[]{
		struct sigaction ours;
		ours.sa_handler = signal_handler;
		return ours;
	}()
}
,sigio_ready{false}
,sigio_caught{false}
{
	mitsein_instance = this;
}


inline
mitsein::~mitsein()
noexcept
{
	sigio_disable();
}


inline
bool mitsein::add(dasein &dasein)
{
	queue.emplace_back(&dasein);
	sort_queue();
	ereignis.notify();
	return true;
}


inline
bool mitsein::cancel(struct dasein &dasein)
{
	const auto it(std::find(begin(queue), end(queue), &dasein));
	if(it == end(queue))
		return false;

	queue.erase(it);
	return true;
}


inline
void mitsein::sort_queue()
{
	std::sort(begin(queue), end(queue), []
	(const auto &a, const auto &b)
	{
		return a->cpu_nice < b->cpu_nice? true:
		       a < b?                     true:
		                                  false;
	});

	auto e(std::unique(begin(queue), end(queue)));
	queue.erase(e, end(queue));
}


inline
void mitsein::operator()(const closure &lambda)
{
	if(queue.empty())
		return;

	sigio_caught = false;
	sigio_enable();
	sigio_install();
	const scope uninstall([this]
	{
		sigio_uninstall();
	});

	for(size_t i(0); i < queue.size(); ++i)
	{
		auto &dasein(*queue.front());
		queue.pop_front();
		if(!lambda(dasein))
			break;
	}
}


inline
//TODO: Need to do all the uplinks and other interesting timerfd's etc
bool mitsein::sigio_enable()
{
	if(!uplink_ready() || sigio_ready)
		return false;

	const int &fd(curr_uplink->conn->pollable->fd);
	const int flags(syscall(::fcntl, fd, F_GETFL));
	syscall(::sigaction, SIGIO, &sa_sigio[IGNORE], &sa_sigio[ORIGINAL]);
	syscall(::fcntl, fd, F_SETOWN, getpid());
	syscall(::fcntl, fd, F_SETFL, flags | FASYNC);
	sigio_ready = true;
	return true;
}


inline
bool mitsein::sigio_disable()
{
	if(!uplink_ready() || !sigio_ready)
		return false;

	const int &fd(curr_uplink->conn->pollable->fd);
	const int flags(syscall(::fcntl, fd, F_GETFL));
	syscall(::fcntl, fd, F_SETFL, flags & ~FASYNC);
	syscall(::sigaction, SIGIO, &sa_sigio[ORIGINAL], nullptr);
	sigio_ready = false;
	return true;
}


inline
void mitsein::sigio_install()
{
	syscall(::sigaction, SIGIO, &sa_sigio[OURS], &sa_sigio[THEIRS]);
}


inline
void mitsein::sigio_uninstall()
{
	syscall(::sigaction, SIGIO, &sa_sigio[THEIRS], nullptr);
}


inline
bool mitsein::uplink_ready()
{
	if(!curr_uplink || !curr_uplink->conn || !curr_uplink->conn->pollable)
		return false;

	if(curr_uplink->conn->pollable->fd <= 0)
		return false;

	return true;
}


inline
void mitsein::signal_handler(int)
noexcept
{
	mitsein_instance->sigio_caught = true;
	std::atomic_signal_fence(std::memory_order_release);
}
