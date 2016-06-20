/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
auto call(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Function> &func,
          const v8::Local<v8::Value> &that,
          std::vector<v8::Local<v8::Value>> &argv)
{
	const context_scope context_scope(ctx);
	return maybe([&ctx, &func, &that, &argv]
	{
		return func->Call(ctx, that, argv.size(), argv.data());
	});
}


inline
auto call(const v8::Local<v8::Context> &ctx,
          const v8::Local<v8::Object> &obj,
          const v8::Local<v8::Value> &that,
          std::vector<v8::Local<v8::Value>> &argv)
{
	const context_scope context_scope(ctx);
	return maybe([&ctx, &obj, &that, &argv]
	{
		return obj->CallAsFunction(ctx, that, argv.size(), argv.data());
	});
}


inline
auto call(const v8::Local<v8::Context> &ctx,
          v8::Local<v8::Value> callable,
          const v8::Local<v8::Value> &that,
          std::vector<v8::Local<v8::Value>> &argv)
{
	using namespace v8;

	return callable->IsFunction()? call(ctx, callable.As<Function>(), that, argv):
	       callable->IsObject()?   call(ctx, callable.As<Object>(), that, argv):
	                               throw type_error("Value is not callable");
}


template<class T>
auto call(const v8::Local<v8::Context> &ctx,
          const T &callable,
          const v8::Local<v8::Value> &that,
          const std::vector<v8::Local<v8::Value>> &argv = {})
{
	auto copy(argv);
	return call(ctx, callable, that, copy);
}


template<class T,
         class... Argv>
auto callv(const T &callable,
           const v8::Local<v8::Value> &that,
           Argv&&... argv)
{
	return call(ctx(), callable, that, {std::forward<Argv>(argv)...});
}


template<class Key,
         class... Argv>
auto callgv(const Key key,
            Argv&&... argv)
{
	auto val(get(key));
	auto that(val->IsObject()? val : ctx()->Global());
	return call(ctx(), val, that, {std::forward<Argv>(argv)...});
}
