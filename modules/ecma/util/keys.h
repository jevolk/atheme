/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
void keys(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          const std::function<void (v8::Local<v8::Name> key)> &closure)
{
	const context_scope context_scope(ctx);
	const auto mebe_keys(obj->GetPropertyNames(ctx));
	if(!mebe_keys)
		return;

	auto keys(checked(mebe_keys));
	for(size_t i(0); has(ctx, keys, i); ++i)
		closure(get<v8::Name>(ctx, keys, i));
}


inline
void keys(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          const std::function<void (const char *const &key)> &closure)
{
	keys(ctx, obj, [&closure]
	(const v8::Local<v8::Name> &key)
	{
		string(key, closure);
	});
}


inline
void keys(const v8::Local<v8::Object> &obj,
          const std::function<void (v8::Local<v8::Name> key)> &closure)
{
	keys(ctx(), obj, closure);
}


inline
void keys(const v8::Local<v8::Object> &obj,
          const std::function<void (const char *const &)> &closure)
{
	keys(obj, [&closure]
	(const v8::Local<v8::Name> &key)
	{
		string(key, closure);
	});
}


//TODO: own() overload over obj
inline
void keys_own(const v8::Local<v8::Context> &ctx,
              const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<v8::Name> key)> &closure)
{
	const context_scope context_scope(ctx);
	const auto mebe_keys(obj->GetOwnPropertyNames(ctx));
	if(!mebe_keys)
		return;

	auto keys(checked(mebe_keys));
	for(size_t i(0); has(ctx, keys, i); ++i)
		closure(get<v8::Name>(ctx, keys, i));
}


//TODO: own() overload over obj
inline
void keys_own(const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<v8::Name> key)> &closure)
{
	return keys_own(ctx(), obj, closure);
}


inline
size_t num_keys(const v8::Local<v8::Context> &ctx,
                const v8::Local<v8::Object> &src)
{
	const context_scope context_scope(ctx);
	const auto keys(src->GetPropertyNames(ctx));
	return test(keys)? checked(keys)->Length() : 0;
}


inline
size_t num_keys(const v8::Local<v8::Object> &src)
{
	return num_keys(ctx(), src);
}


//TODO: own() overload over obj
inline
size_t num_keys_own(const v8::Local<v8::Context> &ctx,
                    const v8::Local<v8::Object> &src)
{
	const context_scope context_scope(ctx);
	const auto keys(src->GetOwnPropertyNames(ctx));
	return test(keys)? checked(keys)->Length() : 0;
}


//TODO: own() overload over obj
inline
size_t num_keys_own(const v8::Local<v8::Object> &src)
{
	return num_keys(ctx(), src);
}
