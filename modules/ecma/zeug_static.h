/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct static_init
{
	static_init();
	~static_init();
};


inline
static_init::static_init()
{
	v8::V8::InitializeICU();
	v8::V8::Initialize();
}


inline
static_init::~static_init()
{
	v8::V8::Dispose();
}
