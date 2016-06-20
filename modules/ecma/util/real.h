/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


// Desert the Real*
using RealNamed = ECMA_STRONG_T(v8::Local<v8::Name>)
using RealIndex = ECMA_STRONG_T(uint32_t)


inline
RealNamed real(const v8::Local<v8::String> &key)
{
	return { key };
}


inline
RealIndex real(const uint32_t &idx)
{
	return { idx };
}


inline
auto real(const char *const &key)
{
	return real(string(key));
}
