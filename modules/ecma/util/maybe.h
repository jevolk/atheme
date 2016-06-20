/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class exception = js_error,
         class T>
v8::Local<T>
maybe(v8::MaybeLocal<T> m,
      const char *const &reason = "")
{
	return test(m)? checked(m) : throw exception(reason);
}


template<class exception = js_error,
         class T>
v8::Local<T> &
maybe(v8::Local<T> &m,
      const char *const &reason = "")
{
	return test(m)? m : throw exception(reason);
}


template<class exception = js_error,
         class T>
v8::Local<T>
maybe(const v8::Local<T> &m,
      const char *const &reason = "")
{
	if(!m)
		throw exception(reason);

	return m;
}


template<class exception = js_error>
void maybe(v8::TryCatch &tc)
{
	if(tc.HasCaught())
		throw exception(tc.Message());
}


template<class exception = js_error,
         class T>
auto maybe(v8::TryCatch &tc,
           v8::MaybeLocal<T> &ml)
{
	if(tc.HasCaught())
		throw exception(tc.Message());

	if(unlikely(!ml))
		throw exception("maybe not.");

	return checked(ml);
}


template<class exception = js_error,
         class lambda>
auto maybe(lambda&& closure)
{
	v8::TryCatch tc;
	auto ret(closure());
	return !tc.HasCaught()? maybe<exception>(ret) : throw exception(tc.Message());
}


template<class T>
auto maybe_empty(v8::MaybeLocal<T> a)
{
	return test(a)? checked(a) : v8::Local<T>{};
}
