/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class R = v8::Object>
auto ctor(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          std::vector<v8::Local<v8::Value>> &argv)
{
	const context_scope context_scope(ctx);
	return as<R>(maybe([&ctx, &obj, &argv]
	{
		return obj->CallAsConstructor(ctx, argv.size(), argv.data());
	}));
}


template<class R = v8::Object>
auto ctor(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          const std::vector<v8::Local<v8::Value>> &argv)
{
	auto copy(argv);
	return ctor<R>(ctx, obj, copy);
}


template<class R = v8::Object,
         class... Argv>
auto ctorv(const v8::Local<v8::Object> &obj,
           Argv&&... argv)
{
	return ctor<R>(ctx(), obj, {std::forward<Argv>(argv)...});
}


template<class R = v8::Object,
         class Key,
         class... Argv>
auto ctorgv(const Key key,
            Argv&&... argv)
{
	auto obj(get<v8::Object>(key));
	return ctor<R>(ctx(), obj, {std::forward<Argv>(argv)...});
}
