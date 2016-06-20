/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const v8::Local<v8::Name> &key)
{
	const context_scope context_scope(ctx);
	return test(obj->Has(ctx, key));
}


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const uint32_t &key)
{
	const context_scope context_scope(ctx);
	return test(obj->Has(ctx, key));
}


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Private &key)
{
	const context_scope context_scope(ctx);
	return test(obj->HasPrivate(ctx, key));
}


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Own &key)
{
	const context_scope context_scope(ctx);
	return test(obj->HasOwnProperty(ctx, key));
}


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const RealNamed &key)
{
	const context_scope context_scope(ctx);
	return test(obj->HasRealNamedProperty(ctx, key));
}


inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const RealIndex &key)
{
	const context_scope context_scope(ctx);
	return test(obj->HasRealIndexedProperty(ctx, key));
}


/*
inline
bool has(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const char *const &key)
{
	return has(ctx, obj, string(key));
}
*/

bool has(const v8::Local<v8::Context> &ctx,
         v8::Local<v8::Object> obj,
         const char *const &key)
{
	using namespace v8;

	bool ret(false);
	size_t i(0);
	const size_t count(tokens_count(key, "."));
	const context_scope context_scope(ctx);
	path(key, [&i, &count, &ctx, &obj, &ret]
	(Local<Name> key)
	{
		if(!test(obj))
			return;
		else if(i == count - 1)
			ret = test(obj->Has(ctx, key));
		else if(!obj->Has(ctx, key))
			obj.Clear();
		else
		{
			auto val(checked(obj->Get(ctx, key)));
			if(!val->IsObject())
				obj.Clear();
			else
				obj = val.As<Object>();
		}

		++i;
	});

	return ret;
}


template<class Key>
bool has(v8::Local<v8::Value> val,
         const Key &key)
{
	if(!val || !val->IsObject())
		return false;

	const auto obj(val.As<v8::Object>());
	return has(ctx(), obj, key);
}


template<class Key,
         class T>
bool has(const v8::PropertyCallbackInfo<T> &info,
         const Key &key)
{
	return has(current(info), instance(info), key);
}


template<class Key,
         class T>
bool has(const v8::FunctionCallbackInfo<T> &info,
         const Key &key)
{
	return has(current(info), instance(info), key);
}


template<class Key>
auto has(const Key &key)
{
	return has(ctx(), ctx()->Global(), key);
}
