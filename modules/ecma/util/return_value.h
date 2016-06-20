/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T,
         class R>
void return_value(const v8::FunctionCallbackInfo<T> &info,
                  R&& value)
{
	info.GetReturnValue().Set(std::move(value));
}


template<class T,
         class R>
void return_value(const v8::PropertyCallbackInfo<T> &info,
                  R&& value)
{
	info.GetReturnValue().Set(std::move(value));
}
