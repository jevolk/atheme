/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include <assert.h>
#include <sys/eventfd.h>
#include <exception>
#include <algorithm>
#include <functional>
#include <atomic>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>

// V8 library
#include "include/libplatform/libplatform.h"
#include "include/v8.h"

// Atheme library
extern "C"
{
	#include <atheme.h>   // -I../../include
	#include <uplink.h>   // -I../../include
	#undef object
	#undef user
}

// Standard aliases
namespace ph = std::placeholders;
using std::begin;
using std::end;
using std::get;

// Convenience globals
service_t *myservice;

#include "util/util.h"                           // Utilities to bring v8 down to the mere mortal masses
#include "LS1.h"                                 // Even more objects to deliver v8 out of bondage
namespace zeug                                   // Equipment
{
	#include "zeug_static.h"                     // Initializer and destructor of v8's globals
	#include "zeug_platform.h"                   // Singleton description of this platform
	#include "zeug_counters.h"                   // Counter state used in the engine
	#include "zeug_stats.h"                      // Collection of all stats/counters attached to zeug
	#include "zeug_opts.h"                       // Collection of options attached to zeug
	#include "zeug.h"                            // Primary v8 engine wrapper
}
using zeug::zeug;
namespace script
{
	using namespace v8;
	#include "script.h"                          // Compiler / Scripting utils
	#include "script_literal.h"                  // Scripts from C string literals to inject into js
}
#include "factory.h"                             // Interface to implement js objects in C
namespace function
{
	using namespace v8;
	#include "function.h"                        // namespace function:: head
	#include "function_literal.h"                // Functions from C string literals to inject into js
	#include "function_generator.h"              // Generator functions from C string literals
	#include "function_factory.h"                // Interface to implement js functions in C
	#include "function_inlined.h"                // Inject a C function/lambda (iface to function::factory)
}
#include "welt.h"                                // 3tz 7adaat 6ovara
#include "object.h"                              // Factory interface to manage C object w/ instance
#include "structure.h"                           // Convenience interface to map C structure to js object.
#include "policy.h"                              // Policy attributes
#include "ereignis.h"                            // Event semaphore
#include "geworfenheit.h"                        // Primal program function
#include "gelassenheit.h"                        // Generator state for the yield/next() dialectic.
#include "erschlossenheit.h"                     // Instruction tape
#include "lichtung.h"                            // Data tape
#include "dasein.h"                              //
#include "zuhanden.h"                            // Running
#include "vorhanden.h"                           // Available
#include "mitsein.h"                             // Scheduler
#include "rede.h"                                // Descriptors

using literal_lib = std::map<std::string, function::literal *>;
using hook_parser = void (&)(void *const &, function::argv &);
using hookv_target = std::function<void (const char *const &name, void *const &data, hook_parser &)>;
using input_target = std::function<void (sourceinfo_t *const &, const char *const &)>;
using sys_target = std::function<v8::Local<v8::Value> (v8::Local<v8::Object> &task, v8::Local<v8::Value> &result)>;
using service_ptr = std::unique_ptr<service_t, void (*)(service_t *)>;
struct ECMA
{
	module_t *module;                            // Atheme module tag;
	service_ptr svc;                             // Atheme service tag
	struct zeug zeug;                            // Main v8 engine instance
	struct welt welt;                            // Global root `this' object.
	struct ereignis ereignis;                    // Event semaphore and timer queue
	struct mitsein mitsein;                      // Scheduler of tasks
	input_target input;                          // User keyboard input
	hookv_target hookv;                          // Atheme hook target
	sys_target star;                             // System software callgate
	literal_lib lib;                             // Registry of js from various modules (read by lib.c)

	bool add(const char *const &name, function::factory *const &obj);
	bool add(const char *const &name, struct welt *const &obj);
	bool del(const char *const &name);

	bool add(function::literal *const &);
	bool del(function::literal *const &);

	template<class... argv> v8::Local<v8::Value> enter(vorhanden::task &, argv&&...);
	template<class... argv> v8::Local<v8::Value> enter(const pid_t &, argv&&...);

	ECMA(module_t *const &m);
	~ECMA();
};


ECMA *ecma;


template<class... argv>
v8::Local<v8::Value>
ECMA::enter(const pid_t &pid,
            argv&&... a)
{
	auto task(vorhanden::get(pid));
	return enter(task, std::forward<argv>(a)...);
}


template<class... argv>
v8::Local<v8::Value>
ECMA::enter(vorhanden::task &task,
            argv&&... a)
{
	auto ctx(vorhanden::ctx(task));
	auto global(vorhanden::global(task));
	return ::call(ctx, task, global, {std::forward<argv>(a)...});
}


inline
bool ECMA::del(function::literal *const &func)
{
	return lib.erase(func->name);
}


inline
bool ECMA::add(function::literal *const &func)
{
	const auto iit(lib.emplace(func->name, func));
	return iit.second;
}


inline
bool ECMA::del(const char *const &name)
try
{
	bool ret{false};
	ret |= welt.child_del(name);
	ret |= welt.func_del(name);
	return ret;
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Failed to del [%s]: %s", name, e.what());
	return false;
}


inline
bool ECMA::add(const char *const &name,
               struct welt *const &obj)
try
{
	return welt.child_add(name, obj);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Failed to add [%s]@%p: %s", name, (const void *)obj, e.what());
	return false;
}


inline
bool ECMA::add(const char *const &name,
               function::factory *const &obj)
try
{
	return welt.func_add(name, obj);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Failed to add [%s]@%p: %s", name, (const void *)obj, e.what());
	return false;
}
