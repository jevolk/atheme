/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T = v8::Value>
auto empty()
{
	return v8::Local<T>{};
}
