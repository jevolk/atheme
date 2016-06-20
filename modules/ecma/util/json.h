/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
size_t json(char *const &buf,
            const size_t &max,
            v8::Local<v8::Context> ctx,
            v8::Local<v8::Object> obj)
{
	if(unlikely(!max))
		return 0;

	size_t ret(0);
	string(maybe(v8::JSON::Stringify(ctx, obj)), [&buf, &max, &ret]
	(const char *const &str)
	{
		ret = mowgli_strlcpy(buf, str, max);
	});

	return ret;
}


inline
const char *json(v8::Local<v8::Context> ctx,
                 v8::Local<v8::Object> obj)
{
	return string(maybe(v8::JSON::Stringify(ctx, obj)));
}


inline
v8::Local<v8::Value>
json(v8::Local<v8::Context> ctx,
     v8::Local<v8::String> input)
{
	return maybe<syntax_error>(v8::JSON::Parse(ctx, input));
}


inline
v8::Local<v8::Value>
json(v8::Local<v8::Context> ctx,
     const char *const &input)
{
	return json(ctx, string(input));
}


inline
v8::Local<v8::Value>
json(v8::Local<v8::String> input)
{
	return json(ctx(), input);
}


inline
v8::Local<v8::Value>
json(const char *const &input)
{
	return json(string(input));
}
