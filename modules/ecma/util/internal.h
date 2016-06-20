/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

// Internal isn't contextual.
// It exists for the factory itself.


inline
void internal(const v8::Local<v8::Object> &obj,
              const int &index,
              const v8::Local<v8::Value> &value)
{
	obj->SetInternalField(index, value);
}


inline
void internal(const v8::Local<v8::Object> &obj,
              const int &index,
              void *const &ptr)
{
	internal(obj, index, v8::External::New(isolate(), ptr));
}


inline
auto internal(const v8::Local<v8::Object> &obj,
              const int &index)
{
	return obj->GetInternalField(index);
}


template<class T = void>
T *internal_ptr(const v8::Local<v8::Object> &obj,
                const int &index)
{
	const auto val(internal(obj, index).As<v8::External>());
	return reinterpret_cast<T *>(val->Value());
}


inline
void reserve_fields(const v8::Local<v8::ObjectTemplate> &obj,
                    const size_t &count)
{
	const size_t cur(obj->InternalFieldCount());
	if(cur < count)
		obj->SetInternalFieldCount(count);
}
