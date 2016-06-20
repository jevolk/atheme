/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class R,
         class T>
R &privdata(const v8::FunctionCallbackInfo<T> &info)
{
	using namespace v8;

	Local<Value> dat(info.Data());
	Local<External> ext(dat.As<External>());
	auto *const ret(static_cast<R *>(ext->Value()));
	return *ret;
}


template<class R,
         class T>
R &privdata(const v8::PropertyCallbackInfo<T> &info)
{
	using namespace v8;

	Local<Value> dat(info.Data());
	Local<External> ext(dat.As<External>());
	auto *const ret(static_cast<R *>(ext->Value()));
	return *ret;
}


template<class R,
         class T>
R &privdata(const v8::WeakCallbackInfo<T> &info,
            const int &index = 0)
{
	const auto ret(static_cast<R *>(info.GetInternalField(index)));
	return *ret;
}
