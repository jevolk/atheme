/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct zeug
{
	using isolate_ptr = std::unique_ptr<v8::Isolate, void (*)(v8::Isolate *)>;
	using interrupt_handler = std::function<void (zeug &)>;
	using microtask_function = std::function<void ()>;

	static int *counter_lookup(const char *const name);
	static void jit_event_handler(const v8::JitCodeEvent *);
	static void entry_hook(uintptr_t addr, uintptr_t retaddr);
	static void event_log(const char *name, int event);
	static void microtasks_completed(v8::Isolate *);
	static void microtask_handler(void *priv);
	static void interrupted(v8::Isolate *, void *priv);
	static void fatal(const char *loc, const char *msg);
	static bool aborted(v8::Isolate *);

	struct platform platform;
	struct static_init static_init;              // Static initialization and destruction for v8 (static_init.h)
	struct v8allocator v8allocator;              // Allocator used for this engine (allocator.h)
	isolate_ptr isolate;                         // Primary resident pointer for the v8::Isolate.
	struct stats stats;
	struct opts opts;

	operator const v8::Isolate *() const         { return isolate.get();                           }
	operator v8::Isolate *()                     { return isolate.get();                           }

	enum field                                   { THIS,                                           };
	template<class T = void> T *get(const field &);
	void set(const field &, void *const &data);

	bool is_dead()                               { return isolate->IsDead();                       }
	bool is_locked()                             { return v8::Locker::IsLocked(isolate.get());     }
	bool in_context()                            { return isolate->InContext();                    }
	bool is_terminating()                        { return isolate->IsExecutionTerminating();       }

	size_t mem_object_stats_string(char *const &buf, const size_t &max, const size_t &idx);
	size_t mem_space_stats_string(char *const &buf, const size_t &max, const size_t &idx);
	size_t mem_heap_stats_string(char *const &buf, const size_t &max);
	void mem_pressure(const v8::MemoryPressureLevel = v8::MemoryPressureLevel::kNone);
	int64_t mem_adjust(const int64_t &delta = 0);
	void mem_stack_limit(uintptr_t limit)        { isolate->SetStackLimit(limit);                  }
	void mem_low()                               { isolate->LowMemoryNotification();               }

	void microtask(v8::Local<v8::Function>);
	void microtask(microtask_function);
	auto microtasks_policy() const               { return isolate->GetMicrotasksPolicy();          }
	void microtasks_policy(const auto &policy)   { isolate->SetMicrotasksPolicy(policy);           }
	void microtasks_run()                        { isolate->RunMicrotasks();                       }

	auto exception(v8::Local<v8::Value> e)       { return isolate->ThrowException(e);              }
	void interrupt(interrupt_handler);
	void interrupt();
	void terminate()                             { isolate->TerminateExecution();                  }
	void resume()                                { isolate->CancelTerminateExecution();            }

	zeug();

	static zeug &get(v8::Isolate *const &);      // Extract an instance with only an isolate*
	static zeug &get();                          // Extract instance with the current thread isolate
};


inline
zeug::zeug()
:isolate{[this]
{
	v8::Isolate::CreateParams cp;
	cp.array_buffer_allocator = &v8allocator;
	//cp.entry_hook = entry_hook;
	//cp.code_event_handler = jit_event_handler;
	return v8::Isolate::New(cp);
}(),[]
(v8::Isolate *const isolate)
{
	isolate->Dispose();
}}
{
	*isolate_extern = isolate.get();
	const isolate_scope isolate_scope;
	isolate->SetCounterFunction(counter_lookup);
	isolate->SetAbortOnUncaughtExceptionCallback(aborted);
	isolate->SetEventLogger(event_log);
	isolate->AddMicrotasksCompletedCallback(microtasks_completed);
	//isolate->SetJitCodeEventHandler(v8::JitCodeEventOptions(0), jit_event_handler);
	set(THIS, this);
}


inline
void zeug::interrupt()
{
	isolate->RequestInterrupt(interrupted, nullptr);
}


inline
void zeug::interrupt(interrupt_handler handler)
{
	const auto copy(new interrupt_handler(std::move(handler)));
	isolate->RequestInterrupt(interrupted, copy);
}


inline
void zeug::microtask(microtask_function function)
{
	const auto copy(new microtask_function(std::move(function)));
	isolate->EnqueueMicrotask(microtask_handler, copy);
}


inline
void zeug::microtask(v8::Local<v8::Function> function)
{
	isolate->EnqueueMicrotask(function);
}


inline
int64_t zeug::mem_adjust(const int64_t &delta)
{
	return isolate->AdjustAmountOfExternalAllocatedMemory(delta);
}


inline
void zeug::mem_pressure(const v8::MemoryPressureLevel level)
{
	isolate->MemoryPressureNotification(level);
}


inline
size_t zeug::mem_heap_stats_string(char *const &buf,
                                   const size_t &max)
{
	v8::HeapStatistics hs;
	isolate->GetHeapStatistics(&hs);
	return heap_stats_string(buf, max, hs);
}


inline
size_t zeug::mem_space_stats_string(char *const &buf,
                                    const size_t &max,
                                    const size_t &idx)
{
	v8::HeapSpaceStatistics hs;
	isolate->GetHeapSpaceStatistics(&hs, idx);
	return heap_stats_string(buf, max, hs);
}


inline
size_t zeug::mem_object_stats_string(char *const &buf,
                                     const size_t &max,
                                     const size_t &idx)
{
	v8::HeapObjectStatistics hs;
	isolate->GetHeapObjectStatisticsAtLastGC(&hs, idx);
	return heap_stats_string(buf, max, hs);
}


inline
void zeug::set(const field &slot,
               void *const &data)
{
	if(unlikely(int(slot) >= isolate->GetNumberOfDataSlots()))
		throw std::out_of_range("slot out of range");

	isolate->SetData(int(slot), data);
}



template<class T>
T *zeug::get(const field &slot)
{
	if(unlikely(int(slot) >= isolate->GetNumberOfDataSlots()))
		throw std::out_of_range("slot out of range");

	void *const ptr(isolate->GetData(int(slot)));
	return reinterpret_cast<T *>(ptr);
}


inline
zeug &zeug::get()
{
	return get(::isolate());
}


inline
zeug &zeug::get(v8::Isolate *const &iso)
{
	void *const ptr(iso->GetData(int(field::THIS)));
	return *static_cast<zeug *>(ptr);
}


inline
bool zeug::aborted(v8::Isolate *const iso)
{
	slog(LG_ERROR, "ABORTION (Uncaught Exception) isolate @ %p", (const void *)iso);
	return true;
}


inline
void zeug::fatal(const char *const location,
                 const char *const message)
{
	slog(LG_ERROR, "----- !!!!! V8 FATAL !!!! ----- @ %s [%s]",
	     location,
	     message);
}


inline
void zeug::interrupted(v8::Isolate *const iso,
                       void *const priv)
{
	zeug &zeug(zeug::get(iso));

	if(!priv)
	{
		zeug.terminate();
		return;
	}

	const auto handler(static_cast<interrupt_handler *>(priv));
	const std::unique_ptr<interrupt_handler> ptr(handler);
	(*handler)(zeug);
}


inline
void zeug::microtask_handler(void *const priv)
{
	const auto function(static_cast<microtask_function *>(priv));
	const std::unique_ptr<microtask_function> ptr(function);
	(*function)();
}


inline
void zeug::microtasks_completed(v8::Isolate *const iso)
{
	printf("microtasks completed %p\n", (const void *)iso);
}


inline
void zeug::event_log(const char *const name,
                     int event)
{
	auto &zeug(zeug::get());
	if(zeug.opts.event_logging)
		slog(LG_DEBUG, "ECMA: (v8 event): [%s] event[%d]", name, event);
}


inline
void zeug::entry_hook(uintptr_t addr,
                      uintptr_t retaddr)
{
	printf("Entry hook: %13lu %13lu\n", addr, retaddr);
}


inline
void zeug::jit_event_handler(const v8::JitCodeEvent *const e)
{
	//auto script(e->script);
	if(e->code_start == 0)
		return;

	printf("JIT code event: type(%d): %-25s start: %p len: %zu priv: %p\n",
	       e->type,
	       reflect(e->type),
	       e->code_start,
	       e->code_len,
	       e->user_data);
}


inline
int *zeug::counter_lookup(const char *const name)
{
	auto &zeug(zeug::get());
	return zeug.stats.counters.lookup(name);
}
