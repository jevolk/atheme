/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

// Disambiguate the obj->Own* mess
using Own = ECMA_STRONG_T(v8::Local<v8::Name>)


inline
Own own(const v8::Local<v8::Name> &key)
{
	return { key };
}


inline
auto own(const char *const &key)
{
	return own(string(key));
}
