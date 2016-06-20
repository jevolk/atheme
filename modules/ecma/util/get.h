/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T = v8::Value>
auto get(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const v8::Local<v8::Name> &key)
{
	const context_scope context_scope(ctx);
	return as<T>(maybe<error<ENOENT>>(obj->Get(ctx, key)));
}


template<class T = v8::Value>
auto get(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const uint32_t &idx)
{
	const context_scope context_scope(ctx);
	return as<T>(maybe<error<ENOENT>>(obj->Get(ctx, idx)));
}


template<class T = v8::Value>
auto get(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Private &key)
{
	const context_scope context_scope(ctx);
	return as<T>(maybe<error<ENOENT>>(obj->GetPrivate(ctx, key)));
}


template<class T = v8::Value>
auto get(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const RealNamed &key)
{
	const context_scope context_scope(ctx);
	return as<T>(maybe<error<ENOENT>>(obj->GetRealNamedProperty(ctx, key)));
}


/*
template<class T = v8::Value>
v8::Local<T>
get(v8::Local<v8::Context> ctx,
    v8::Local<v8::Object> obj,
    const char *const &key)
{
	return get<T>(ctx, obj, string(key));
}
*/

template<class T = v8::Value>
auto get(const v8::Local<v8::Context> &ctx,
         v8::Local<v8::Object> obj,
         const char *const &key)
{
	using namespace v8;

	size_t i(0);
	MaybeLocal<Value> ret;
	const size_t count(tokens_count(key, "."));
	const context_scope context_scope(ctx);
	path(key, [&i, &count, &ctx, &obj, &ret]
	(Local<Name> key)
	{
		if(!test(obj))
			return;
		else if(i == count - 1)
			ret = obj->Get(ctx, key);
		else if(!has(ctx, obj, key))
			obj.Clear();
		else
			obj = get<Object>(ctx, obj, key);

		++i;
	});

	return as<T>(maybe<error<ENOENT>>(ret));
}


template<class T = v8::Value,
         class Key>
auto get(const v8::Local<v8::Object> &obj,
         const Key &key)
{
	return get<T>(ctx(), obj, key);
}


template<class R = v8::Value,
         class Key,
         class T>
auto get(const v8::PropertyCallbackInfo<T> &info,
         const Key &key)
{
	return get<R>(current(info), instance(info), key);
}


template<class R = v8::Value,
         class Key,
         class T>
auto get(const v8::FunctionCallbackInfo<T> &info,
         const Key &key)
{
	return get<R>(current(info), instance(info), key);
}


template<ssize_t N,
         class R = v8::Value,
         class T>
auto get(const v8::FunctionCallbackInfo<T> &info)
try
{
	if(info.Length() <= N)
		throw error<EINVAL>("Missing required argument.");

	return as<R>(info[N]);
}
catch(error<EINVAL> &e)
{
	throw error<EINVAL>("Argument [%zu]: %s", N, e.what());
}


template<class T = v8::Value,
         class Key>
auto get(const Key &key)
{
	return get<T>(ctx(), ctx()->Global(), key);
}
