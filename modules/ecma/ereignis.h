/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

class ereignis
{
	using handler = std::function<void ()>;

	mowgli_eventloop_pollable_t *pollable;
	mowgli_eventloop_timer_t *timer;
	std::deque<time_t> timeouts;
	handler target;
	bool pending;
	bool active;

	static void timer_target(void *) noexcept;
	static void poll_target(mowgli_eventloop_t *, mowgli_eventloop_io_t *, mowgli_eventloop_io_dir_t, void *) noexcept;

	void notify_next();
	void sort();

  public:
	auto is_pending() const                      { return pending;                                 }
	auto is_active() const                       { return active;                                  }

	void notify_absolute(const time_t &time);
	void notify_relative(const time_t &time);
	bool notify();

	void set(const handler &target)              { this->target = target;                          }
	bool deactivate();
	bool activate();

	ereignis(const handler & = {}, const bool &activate = false);
	~ereignis();
};


inline
ereignis::ereignis(const handler &target,
                   const bool &activate)
:pollable{[this]
{
	const auto fd(syscall(::eventfd, 0, EFD_SEMAPHORE | EFD_NONBLOCK));
	const auto ret(mowgli_pollable_create(base_eventloop, fd, this));
	return ret?: throw std::runtime_error("Failed to add eventfd() to the base_eventloop");
}()}
,timer{nullptr}
,target{target}
,pending{false}
,active{false}
{
	if(activate)
		this->activate();
}


inline
ereignis::~ereignis()
noexcept
{
	if(timer)
		mowgli_timer_destroy(base_eventloop, timer);

	mowgli_pollable_destroy(base_eventloop, pollable);
}


inline
bool ereignis::activate()
{
	mowgli_pollable_setselect(base_eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ, poll_target);
	active = true;
	return active;
}


inline
bool ereignis::deactivate()
{
	const bool ret(is_active());
	mowgli_pollable_setselect(base_eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ, NULL);
	active = false;
	return ret;
}


inline
bool ereignis::notify()
{
	if(pending)
		return false;

	const auto &fd(pollable->fd);
	syscall(::eventfd_write, fd, 1);
	pending = true;
	return true;
}


inline
void ereignis::notify_relative(const time_t &reltime)
{
	const auto abstime(CURRTIME + reltime);
	notify_absolute(abstime);
}


inline
void ereignis::notify_absolute(const time_t &abstime)
{
	timeouts.emplace_back(abstime);
	sort();
	notify_next();
}


inline
void ereignis::sort()
{
	std::sort(begin(timeouts), end(timeouts));
	const auto e(std::unique(begin(timeouts), end(timeouts)));
	timeouts.erase(e, end(timeouts));
}


inline
void ereignis::notify_next()
{
	if(timeouts.empty())
		return;

	const auto &next(timeouts.front());
	if(timer && next == timer->deadline)
		return;

	if(timer && timer->deadline > CURRTIME)
		mowgli_timer_destroy(base_eventloop, timer);

	static const auto name("ECMA_ereignis_timer");
	const auto reltime(std::max(next - CURRTIME, 0L));
	timer = mowgli_timer_add_once(base_eventloop, name, timer_target, this, reltime);
}


inline
void ereignis::poll_target(mowgli_eventloop_t *const evl,
                           mowgli_eventloop_io_t *const io,
                           mowgli_eventloop_io_dir_t dir,
                           void *const priv)
noexcept
{
	auto &ereignis(*reinterpret_cast<struct ereignis *>(priv));
	const auto &fd(ereignis.pollable->fd);

	eventfd_t value;
	syscall(::eventfd_read, fd, &value);
	ereignis.pending = false;
	if(likely(bool(ereignis.target)))
		ereignis.target();
}


inline
void ereignis::timer_target(void *const priv)
noexcept
{
	auto &ereignis(*reinterpret_cast<struct ereignis *>(priv));
	assert(!ereignis.timeouts.empty());
	ereignis.timeouts.pop_front();
	const scope next([&ereignis]
	{
		ereignis.sort();
		ereignis.notify_next();
	});

	if(likely(bool(ereignis.target)))
		ereignis.target();
}
