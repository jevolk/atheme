/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool del(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const v8::Local<v8::Name> &key)
{
	const context_scope context_scope(ctx);
	return test(obj->Delete(ctx, key));
}


inline
bool del(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const uint32_t &key)
{
	const context_scope context_scope(ctx);
	return test(obj->Delete(ctx, key));
}


inline
bool del(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Private &key)
{
	const context_scope context_scope(ctx);
	return test(obj->DeletePrivate(ctx, key));
}

/*
inline
bool del(v8::Local<v8::Context> ctx,
         v8::Local<v8::Object> obj,
         const char *const &key)
{
	return del(ctx, obj, string(key));
}
*/

inline
bool del(const v8::Local<v8::Context> &ctx,
         v8::Local<v8::Object> obj,
         const char *const &key)
{
	using namespace v8;

	size_t i(0);
	const size_t count(tokens_count(key, "."));
	path(key, [&i, &count, &ctx, &obj]
	(Local<Name> key)
	{
		if(!test(obj))
			return;

		if(i == count - 1)
		{
			del(ctx, obj, key);
			++i;
		}
		else if(!is_object(ctx, obj, key))
		{
			obj.Clear();
		} else {
			obj = get<Object>(ctx, obj, key);
			++i;
		}
	});

	return i == count;
}


template<class Key>
bool del(const v8::Local<v8::Object> &obj,
         const Key &key)
{
	return del(ctx(), obj, key);
}


template<class Key,
         class T>
bool del(const v8::PropertyCallbackInfo<T> &info,
         const Key &key)
{
	return del(current(info), instance(info), key);
}


template<class Key,
         class T>
bool del(const v8::FunctionCallbackInfo<T> &info,
         const Key &key)
{
	return del(current(info), instance(info), key);
}


template<class Key>
bool del(const Key &key)
{
	return del(ctx(), ctx()->Global(), key);
}
