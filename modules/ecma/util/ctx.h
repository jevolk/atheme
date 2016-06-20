/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
auto our_ctx()
{
	assert(context_scope_current);
	assert(*context_scope_current);
	return (*context_scope_current)->context;
}


inline
auto entered_ctx()
{
	return isolate()->GetEnteredContext();
}


inline
auto current_ctx()
{
	return isolate()->GetCurrentContext();
}


inline
auto ctx()
{
	return current_ctx();
}


inline
auto have_our_ctx()
{
	return bool(*context_scope_current);
}


inline
auto in_ctx()
{
	return isolate()->InContext();
}


inline
auto in_our_ctx()
{
	return ident(current_ctx()) == ident(our_ctx());
}


template<class T>
auto holder(const v8::PropertyCallbackInfo<T> &info)
{
    return info.Holder();
}


template<class T>
auto holder(const v8::FunctionCallbackInfo<T> &info)
{
	return info.Holder();
}


template<class T>
auto instance(const v8::PropertyCallbackInfo<T> &info)
{
    return info.This();
}


template<class T>
auto instance(const v8::FunctionCallbackInfo<T> &info)
{
	return info.This();
}


inline
auto creator(const v8::Local<v8::Object> &obj)
{
	return obj->CreationContext();
}


template<class T>
auto creator(const v8::PropertyCallbackInfo<T> &info)
{
	return creator(holder(info));
}


template<class T>
auto creator(const v8::FunctionCallbackInfo<T> &info)
{
	return creator(holder(info));
}


template<class T>
auto requestor(const v8::PropertyCallbackInfo<T> &info)
{
	return creator(instance(info));
}


template<class T>
auto requestor(const v8::FunctionCallbackInfo<T> &info)
{
	return creator(instance(info));
}


template<class T>
auto current(const v8::PropertyCallbackInfo<T> &info)
{
	return in_ctx()? ctx() : requestor(info);
}


template<class T>
auto current(const v8::FunctionCallbackInfo<T> &info)
{
	return in_ctx()? ctx() : requestor(info);
}


template<class T>
auto interinstance(const v8::PropertyCallbackInfo<T> &info)
{
	return ident(holder(info)) != ident(requestor(info));
}


template<class T>
auto interinstance(const v8::FunctionCallbackInfo<T> &info)
{
	return ident(holder(info)) != ident(requestor(info));
}


template<class T>
auto intercontext(const v8::Local<v8::Context> &ctx,
                  const v8::PropertyCallbackInfo<T> &info)
{
	return ident(ctx) != ident(requestor(info));
}


template<class T>
auto intercontext(const v8::Local<v8::Context> &ctx,
                  const v8::FunctionCallbackInfo<T> &info)
{
	return ident(ctx) != ident(requestor(info));
}


template<class T>
auto intercontext(const v8::PropertyCallbackInfo<T> &info)
{
	return intercontext(current(info), info);
}


template<class T>
auto intercontext(const v8::FunctionCallbackInfo<T> &info)
{
	return intercontext(current(info), info);
}


template<class T>
auto local(v8::Isolate *const &i,
           const v8::Global<T> &g)
{
	return g.Get(i);
}


template<class T>
auto local(const v8::Global<T> &g)
{
	return local(isolate(), g);
}


template<class T>
auto global(const v8::Local<T> &local)
{
	return v8::Global<T>(isolate(), local);
}


inline
auto global()
{
	return ctx()->Global();
}
