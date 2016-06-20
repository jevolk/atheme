/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool is_object(const v8::Local<v8::Context> &ctx,
               const v8::Local<v8::Object> &obj,
               const v8::Local<v8::Name> &key)
{
	return proffer(ctx, obj, key, []
	(v8::Local<v8::Value> val)
	{
		return val->IsObject();
	});
}


inline
bool is_object(const v8::Local<v8::Context> &ctx,
               const v8::Local<v8::Object> &obj,
               const char *const &key)
{
	return is_object(ctx, obj, string(key));
}


template<class Key>
bool is_object(const v8::Local<v8::Object> &obj,
               const Key &key)
{
	return is_object(ctx(), obj, key);
}
