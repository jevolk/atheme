/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool has_interceptor(v8::Local<v8::Value> val)
{
	if(!val->IsObject())
		return false;

	const auto obj(val.As<v8::Object>());
	return obj->HasNamedLookupInterceptor() ||
	       obj->HasIndexedLookupInterceptor();
}


inline
void clear(const v8::Local<v8::Context> &ctx,
           const v8::Local<v8::Object> &obj)
{
	keys(ctx, obj, [&ctx, &obj]
	(v8::Local<v8::Name> key)
	{
		del(ctx, obj, key);
	});
}


inline
void clear(const v8::Local<v8::Object> &obj)
{
	clear(ctx(), obj);
}


template<class Key0,
         class Key1>
void copy(const v8::Local<v8::Context> &dst_ctx,
          const Key0 &dst_key,
          const v8::Local<v8::Object> &dst,
          const v8::Local<v8::Context> &src_ctx,
          const Key1 &src_key,
          const v8::Local<v8::Object> &src)
{
	const auto val(get(src_ctx, src, src_key));
	set(dst_ctx, dst, dst_key, val);
}


template<class Key>
void copy(const v8::Local<v8::Context> &dst_ctx,
          const v8::Local<v8::Context> &src_ctx,
          const Key &key,
          const v8::Local<v8::Object> &val)
{
	copy(dst_ctx, key, val, src_ctx, key, val);
}


inline
void copy(const v8::Local<v8::Context> &dst_ctx,
          const v8::Local<v8::Object> &dst,
          const v8::Local<v8::Context> &src_ctx,
          const v8::Local<v8::Object> &src)
{
	using namespace v8;

	const handle_scope handle_scope;
	for_each<Value>(src_ctx, src, [&dst_ctx, &src_ctx, &dst]
	(Local<Name> &key, Local<Value> val)
	{
		set(dst_ctx, dst, key, val);
	});
}


inline
void copy(const v8::Local<v8::Context> &dst_ctx,
          const v8::Local<v8::Context> &src_ctx,
          const v8::Local<v8::Object> &obj)
{
	copy(dst_ctx, obj, src_ctx, obj);
}


template<class Key>
void copy(const v8::Local<v8::Context> &dst_ctx,
          const v8::Local<v8::Context> &src_ctx,
          const Key &key)
{
	copy(dst_ctx, key, dst_ctx->Global(), src_ctx, key, src_ctx->Global());
}


template<class Key0,
         class Key1>
void copy(const v8::Local<v8::Context> &dst_ctx,
          const Key0 &dst_key,
          const v8::Local<v8::Context> &src_ctx,
          const Key1 &src_key)
{
	copy(dst_ctx, dst_key, dst_ctx->Global(), src_ctx, src_key, src_ctx->Global());
}


inline
void copy_own(const v8::Local<v8::Context> &dst_ctx,
              const v8::Local<v8::Object> &dst,
              const v8::Local<v8::Context> &src_ctx,
              const v8::Local<v8::Object> &src)
{
	using namespace v8;

	const handle_scope handle_scope;
	for_each_own<Value>(src_ctx, src, [&dst_ctx, &src_ctx, &dst]
	(Local<Name> &key, Local<Value> val)
	{
		set(dst_ctx, dst, key, val);
	});
}


template<class Key0,
         class Key1>
void rename(const v8::Local<v8::Context> &ctx,
            const v8::Local<v8::Object> &obj,
            const Key0 &from,
            const Key1 &to)
{
	set(ctx, obj, to, get(ctx, obj, from));
	del(ctx, obj, from);
}


template<class Key0,
         class Key1>
void rename(const v8::Local<v8::Object> &obj,
            const Key0 &from,
            const Key1 &to)
{
	rename(ctx(), obj, from, to);
}


template<class Key0,
         class Key1>
void rename(const v8::Local<v8::Context> &ctx,
            const Key0 &from,
            const Key1 &to)
{
	rename(ctx, ctx->Global(), from, to);
}


template<class iterator>
void copy_globals(iterator src,
                  iterator src_end,
                  iterator dst)
{
	for(; src != src_end; ++src, ++dst)
		dst->Reset(isolate(), local(*src));
}


inline
v8::Local<v8::Array>
array(const v8::Local<v8::Context> &ctx,
      std::vector<v8::Local<v8::Value>> &vec)
{
	using namespace v8;

	const context_scope context_scope(ctx);
	Local<Array> ret(Array::New(isolate(), vec.size()));
	for(size_t i(0); i < vec.size(); ++i)
		set(ctx, ret, i, vec[i]);

	return ret;
}


inline
v8::Local<v8::Array>
array(std::vector<v8::Local<v8::Value>> &vec)
{
	return array(ctx(), vec);
}
