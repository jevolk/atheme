/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

namespace rede
{
	using namespace v8;

	Local<Object> fds();

	Local<Object> get(const uint64_t &id);
	Local<Context> ctx(const uint64_t &id);

	Local<Object> described(Local<Object> descriptor);
	Local<Object> described(const uint64_t &id);

	void result(Local<Object> descriptor, Local<Value> val);
	void result(const uint64_t &id, Local<Value> value);

	Local<Object> open(Local<Object> described = {});
	void close(Local<Object> descriptor);
	void close(const uint64_t &id);
}


inline
void rede::close(const uint64_t &id)
{
	::del(fds(), id);
}


inline
void rede::close(Local<Object> descriptor)
{
	const auto id(::get<uint64_t>(descriptor, "id"));
	::del(fds(), id);
}


inline
v8::Local<v8::Object>
rede::open(Local<Object> described)
{
	LS1::integer pid(dasein::getpid(::ctx()));
	return ctorgv("proc.fd", described, pid);
}


inline
void rede::result(const uint64_t &id,
                  Local<Value> value)
{
	result(get(id), value);
}


inline
void rede::result(Local<Object> descriptor,
                  Local<Value> val)
{
	set(descriptor, "result", val);
}


inline
v8::Local<v8::Object>
rede::described(const uint64_t &id)
{
	return described(get(id));
}


inline
v8::Local<v8::Object>
rede::described(Local<Object> descriptor)
{
	return ::get<Object>(descriptor, "object");
}


inline
v8::Local<v8::Context>
rede::ctx(const uint64_t &id)
{
	return creator(get(id));
}


inline
v8::Local<v8::Object>
rede::get(const uint64_t &id)
{
	return ::get<Object>(fds(), id);
}


inline
v8::Local<v8::Object>
rede::fds()
{
	auto proc(::get<Object>("proc"));
	auto fds(::get<Object>(proc, "fd"));
	return fds;
}
