/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct platform
{
	v8::Platform *p;

	platform();
	~platform();
};


inline
platform::platform()
:p{v8::platform::CreateDefaultPlatform(1)}
{
	v8::V8::InitializePlatform(p);
}


inline
platform::~platform()
noexcept
{
	v8::V8::ShutdownPlatform();
	delete p;
}



/*

struct platform
:v8::Platform
{
	size_t NumberOfAvailableBackgroundThreads()
	override
	{
		slog(LG_DEBUG, "NumberOfAvailableBackgroundThreads");
		assert(0);
		return 0;
	}

	void CallOnBackgroundThread(v8::Task *task, ExpectedRuntime expected_runtime)
	override
	{
		slog(LG_DEBUG, "CALL ON BACKGROUND");
		assert(0);
	}

	void CallOnForegroundThread(v8::Isolate *isolate, v8::Task *task)
	override
	{
		slog(LG_DEBUG, "CALL DELAYED ON FOREGROUND");
		assert(0);
	}

	void CallDelayedOnForegroundThread(v8::Isolate *isolate, v8::Task *task, double delay_in_seconds)
	override
	{
		slog(LG_DEBUG, "CALL DELAYED ON FOREGROUND");
		assert(0);
	}

	void CallIdleOnForegroundThread(v8::Isolate *isolate, v8::IdleTask *task)
	override
	{
		slog(LG_DEBUG, "CALL IDLE ON FOREGROUND");
		assert(0);
	}

	bool IdleTasksEnabled(v8::Isolate *isolate)
	override
	{
		//slog(LG_DEBUG, "IdleTasksEnabled");
		return false;
	}

	double MonotonicallyIncreasingTime()
	override
	{
		//slog(LG_DEBUG, "MonotonicallyIncreasingTime");
		//assert(0);
		return CURRTIME;
	}

	const uint8_t *GetCategoryGroupEnabled(const char *name)
	override
	{
		//slog(LG_DEBUG, "GetCategoryGroupEnabled");
		static uint8_t categoryflags[8];
		return categoryflags;
	}

	const char *GetCategoryGroupName(const uint8_t *category_enabled_flag)
	override
	{
		slog(LG_DEBUG, "GetCategoryGroupName");
		assert(0);
		return "none";
	}

	uint64_t AddTraceEvent(char phase, const uint8_t *category_enabled_flag, const char *name, const char *scope, uint64_t id, uint64_t bind_id, int32_t num_args, const char **arg_names, const uint8_t *arg_types, const uint64_t *arg_values, unsigned int flags)
	override
	{
		slog(LG_DEBUG, "AddTraceEvent");
		assert(0);
		return 0;
	}

	void UpdateTraceEventDuration(const uint8_t *category_enabled_flag, const char *name, uint64_t handle)
	override
	{
		slog(LG_DEBUG, "UpdateTraceEvent");
		assert(0);
	}

	platform();
	~platform();
};

*/
