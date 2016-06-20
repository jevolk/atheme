/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const v8::Local<v8::Name> &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->Set(ctx, key, val);
}


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const uint32_t &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->Set(ctx, key, val);
}


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Private &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->SetPrivate(ctx, key, val);
}


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const Own &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->DefineOwnProperty(ctx, key, val);
}


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const DataNamed &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->CreateDataProperty(ctx, key, val);
}


inline
void set(const v8::Local<v8::Context> &ctx,
         const v8::Local<v8::Object> &obj,
         const DataIndex &key,
         const v8::Local<v8::Value> &val)
{
	const context_scope context_scope(ctx);
	obj->CreateDataProperty(ctx, key, val);
}


/*
inline
void set(v8::Local<v8::Context> ctx,
         v8::Local<v8::Object> obj,
         const char *const &key,
         v8::Local<v8::Value> val)
{
	set(ctx, obj, string(key), val);
}
*/

inline
void set(const v8::Local<v8::Context> &ctx,
         v8::Local<v8::Object> obj,
         const char *const &key,
         const v8::Local<v8::Value> &val)
{
	using namespace v8;

	size_t i(0);
	const size_t count(tokens_count(key, "."));
	path(key, [&i, &count, &ctx, &obj, &val]
	(Local<Name> key)
	{
		if(i == count - 1)
		{
			set(ctx, obj, key, val);
			return;
		}

		if(!is_object(ctx, obj, key))
		{
			const context_scope context_scope(ctx);
			auto child(Object::New(isolate()));
			set(ctx, obj, key, child);
		}

		obj = get<Object>(ctx, obj, key);
		++i;
	});
}


template<class Key>
void set(const v8::Local<v8::Object> &obj,
         const Key &key,
         const v8::Local<v8::Value> &val)
{
	set(ctx(), obj, key, val);
}


template<class Key,
         class T>
void set(const v8::PropertyCallbackInfo<T> &info,
         const Key &key,
         const v8::Local<v8::Value> &val)
{
	set(current(info), instance(info), key, val);
}


template<class Key,
         class T>
void set(const v8::FunctionCallbackInfo<T> &info,
         const Key &key,
         const v8::Local<v8::Value> &val)
{
	set(current(info), instance(info), key, val);
}
