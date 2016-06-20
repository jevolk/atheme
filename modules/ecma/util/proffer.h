/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool proffer(const v8::Local<v8::Context> &ctx,
             const v8::Local<v8::Object> &obj,
             const v8::Local<v8::Name> &key,
             const std::function<bool (v8::Local<v8::Value>)> &closure)
{
	if(!has(ctx, obj, key))
		return false;

	return closure(get(ctx, obj, key));
}


inline
bool proffer(const v8::Local<v8::Context> &ctx,
             const v8::Local<v8::Object> &obj,
             const char *const &key,
             const std::function<bool (v8::Local<v8::Value>)> &closure)
{
	return proffer(ctx, obj, string(key), closure);
}


template<class Key>
bool proffer(const v8::Local<v8::Object> &obj,
             const Key &key,
             const std::function<bool (v8::Local<v8::Value>)> &closure)
{
	return proffer(ctx(), obj, key, closure);
}
