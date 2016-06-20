/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
bool dtor(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          const std::vector<v8::Local<v8::Value>> &argv = {})
{
	auto key(priv("dtor"));
	if(!has(ctx, obj, key))
		return false;

	auto func(get<v8::Function>(ctx, obj, key));
	call(ctx, func, obj, argv);
	return true;
}


inline
bool dtor(const v8::Local<v8::Object> &obj,
          const std::vector<v8::Local<v8::Value>> &argv = {})
{
	return dtor(ctx(), obj, argv);
}
