/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 *
 * We want to disambiguate v8::Private from v8::Name for a cleaner strongly typed API.
 *
 * bad:   get_priv(obj, "key");
 * good:  get(obj, priv("key"));
 *
 * (see get.h et al)
 * 
 * The former is a v8::Value and the latter is not so it almost works, but ambiguity
 * still exists because they both inherit from the abstract base v8::Data. We thus return
 * a strong wrapper from priv() to disambiguate overload resolution.
 */

using Private = ECMA_STRONG_T(v8::Local<v8::Private>)


inline
Private priv(const v8::Local<v8::String> &key)
{
	return { v8::Private::ForApi(isolate(), key) };
}


inline
auto priv(const char *const &key)
{
	return priv(string(key));
}
