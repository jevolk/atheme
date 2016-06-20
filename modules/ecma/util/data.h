/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

// Disambiguate the Data
using DataNamed = ECMA_STRONG_T(v8::Local<v8::Name>)
using DataIndex = ECMA_STRONG_T(uint32_t)


inline
DataNamed data(const v8::Local<v8::String> &key)
{
	return { key };
}


inline
DataIndex data(const uint32_t &idx)
{
	return { idx };
}


inline
auto data(const char *const &key)
{
	return real(string(key));
}
