/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T>
auto checked(v8::MaybeLocal<T> a)
{
	return a.ToLocalChecked();
}


inline
bool test(const v8::Maybe<bool> &a)
{
	return a.FromMaybe(false);
}


inline
bool test(v8::Local<v8::Boolean> a)
{
	return !a.IsEmpty() && a->Value();
}


inline
bool test(v8::MaybeLocal<v8::Boolean> a)
{
	return !a.IsEmpty() && checked(a)->Value();
}


template<class T>
bool test(const v8::Maybe<T> &a)
{
	return !a.IsNothing();
}


template<class T>
bool test(const v8::MaybeLocal<T> &a)
{
	return !a.IsEmpty();
}


template<class T>
bool test(const v8::Local<T> &a)
{
	return !a.IsEmpty();
}


template<class T>
bool test(const v8::Global<T> &a)
{
	return !a.IsEmpty();
}


template<class T>
bool operator!(const v8::MaybeLocal<T> &a)
{
	return !test(a);
}


template<class T>
bool operator!(const v8::Local<T> &a)
{
	return !test(a);
}


template<class T>
bool operator!(const v8::Global<T> &a)
{
	return !test(a);
}


template<class T>
bool operator!(const v8::Maybe<T> &a)
{
	return !test(a);
}


template<class T>
bool defined(const v8::MaybeLocal<T> &a)
{
	return test(a) && !checked(a)->IsUndefined();
}


template<class T>
bool defined(const v8::Local<T> &a)
{
	return test(a) && !a->IsUndefined();
}


template<class T>
auto empty_undefined(v8::MaybeLocal<T> a)
{
	return test(a)? checked(a) : v8::Undefined(isolate());
}
