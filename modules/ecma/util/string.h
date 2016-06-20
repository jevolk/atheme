/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
void string(const v8::Local<v8::Value> &val,
            const std::function<void (const char *const &)> &closure)
{
	const v8::String::Utf8Value str(val);
	closure(*str);
}


inline
const char *string(const v8::Local<v8::Value> &val)
{
	static const short BUFS(16);
	thread_local short ctr;
	thread_local char bufs[BUFS][BUFSIZE];

	char *ret;
	string(val, [&ret](const char *const &str)
	{
		ret = bufs[ctr++];
		ctr %= BUFS;
		mowgli_strlcpy(ret, str?: "(null)", BUFSIZE);
	});

	return ret;
}


template<class T>
const char *string(v8::MaybeLocal<T> obj)
{
	return likely(test(obj))? string(checked(obj)) : "(null)";
}


v8::Local<v8::String>
string(const char *const &str)
{
	using namespace v8;

	return String::NewFromUtf8(isolate(), str, String::kNormalString);
}


/*
NO_OPTIONS
HINT_MANY_WRITES_EXPECTED
NO_NULL_TERMINATION
PRESERVE_ONE_BYTE_NULL
REPLACE_INVALID_UTF8

> Copies the contents of the string and the NULL terminator into the buffer.
> WriteUtf8 will not write partial UTF-8 sequences, preferring to stop before the end of the buffer.
*/

/*
inline
size_t strlcpy(char *const buf,
               const v8::Local<v8::String> &str,
               const ssize_t &max,
               v8::String::WriteOptions opts = v8::String::WriteOptions::NO_OPTIONS)
{
	if(unlikely(!max))
		return 0;

	if(unlikely(!str))
		return mowgli_strlcpy(buf, "(null)", max);

	int written(0);
	ssize_t ret(str->WriteUtf8(buf, max, &written, opts));
	assert(ret == written);

	if(ret == max)
	{
		--ret;
		buf[ret] = '\0';
	}

	return written;
}
*/
