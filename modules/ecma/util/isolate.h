/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

v8::Isolate *isolate();
template<class T> v8::Isolate *isolate(const v8::PropertyCallbackInfo<T> &info);
template<class T> v8::Isolate *isolate(const v8::FunctionCallbackInfo<T> &info);
template<class T> v8::Isolate *isolate(const v8::WeakCallbackInfo<T> &info);
v8::Isolate *isolate(v8::Local<v8::Context> ctx);

struct isolate_scope
{
	struct exit
	{
		v8::Unlocker unlocker;

		exit();
		~exit();
	};

	v8::Locker locker;

	isolate_scope();
	~isolate_scope();
};

extern "C"
{
	v8::Isolate **isolate_extern;
}


inline
isolate_scope::isolate_scope()
:locker{*isolate_extern}
{
	(*isolate_extern)->Enter();
}


inline
isolate_scope::~isolate_scope()
{
	(*isolate_extern)->Exit();
}


inline
isolate_scope::exit::exit()
:unlocker{*isolate_extern}
{
}


inline
isolate_scope::exit::~exit()
{
}


inline
v8::Isolate *isolate(v8::Local<v8::Context> ctx)
{
	return ctx->GetIsolate();
}


template<class T>
v8::Isolate *isolate(const v8::WeakCallbackInfo<T> &info)
{
	return info.GetIsolate();
}


template<class T>
v8::Isolate *isolate(const v8::FunctionCallbackInfo<T> &info)
{
	return info.GetIsolate();
}


template<class T>
v8::Isolate *isolate(const v8::PropertyCallbackInfo<T> &info)
{
	return info.GetIsolate();
}


inline
v8::Isolate *isolate()
{
	//assert(isolate_scope_current);
	//assert(*isolate_scope_current);
	//return (*isolate_scope_current)->ours;
	return v8::Isolate::GetCurrent();
}
