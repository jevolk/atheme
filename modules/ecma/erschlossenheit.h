/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct erschlossenheit
{
	using argv = function::argv;

	struct geworfenheit geworfenheit;            // Original main function
	struct gelassenheit gelassenheit;            // Generator state for the yield/next() dialectic

	v8::MaybeLocal<v8::Value> operator()(const v8::Local<v8::Object> &global, argv &);
};


inline
v8::MaybeLocal<v8::Value>
erschlossenheit::operator()(const v8::Local<v8::Object> &that,
                            argv &a)
{
	if(gelassenheit)
	{
		auto next(gelassenheit.get_next());
		return next->Call(ctx(), gelassenheit, a.size(), a.data());
	}

	if(unlikely(!geworfenheit))
		return {};

	v8::Local<v8::Function> function(geworfenheit);
	return function->Call(ctx(), that, a.size(), a.data());
}
