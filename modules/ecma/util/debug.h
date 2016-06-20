/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


/* For conforming functions:
 *
 * make:  size_t my_debug(char *buf, size_t max, foo_t foo, bar_t bar) { }
 * then:  slog_debug(my_debug, foo, bar);
 */
template<class func,
         class... args>
static
void slog_debug(const func &f,
                args&&... a)
{
	static thread_local char buf[BUFSIZE];
	f(buf, sizeof(buf), std::forward<args>(a)...);
	slog(LG_DEBUG, "%s", buf);
}


inline
ssize_t debug_stack_frame(char *const &buf,
                          const ssize_t &max,
                          const v8::StackFrame &frame)
{
	const handle_scope handle_scope;
	return snprintf(buf, max, "[%-5u %-18s %c%c] @%3u +%-3u => %s",
	                frame.GetScriptId(),
	                string(frame.GetScriptName()),
	                frame.IsEval()? 'E' : '-',
	                frame.IsConstructor()? 'C' : '-',
	                frame.GetLineNumber(),
	                frame.GetColumn(),
	                string(frame.GetFunctionName()));
}


inline
ssize_t debug_value(char *const &buf,
                    const ssize_t &max,
                    v8::Local<v8::Value> val)
{
	if(max <= 0)
		return 0;

	if(val.IsEmpty())
		return mowgli_strlcpy(buf, "<EMPTY HANDLE>", max);

	buf[0] = '\0';
	mowgli_strlcat(buf, "[", max);

	if(val->IsUndefined())          mowgli_strlcat(buf, " Undefined", max);
	if(val->IsNull())               mowgli_strlcat(buf, " Null", max);
	if(val->IsTrue())               mowgli_strlcat(buf, " True", max);
	if(val->IsFalse())              mowgli_strlcat(buf, " False", max);
	if(val->IsName())               mowgli_strlcat(buf, " Name", max);
	if(val->IsString())             mowgli_strlcat(buf, " String", max);
	if(val->IsSymbol())             mowgli_strlcat(buf, " Symbol", max);
	if(val->IsFunction())           mowgli_strlcat(buf, " Function", max);
	if(val->IsArray())              mowgli_strlcat(buf, " Array", max);
	if(val->IsObject())             mowgli_strlcat(buf, " Object", max);
	if(val->IsBoolean())            mowgli_strlcat(buf, " Boolean", max);
	if(val->IsNumber())             mowgli_strlcat(buf, " Number", max);
	if(val->IsExternal())           mowgli_strlcat(buf, " External", max);
	if(val->IsInt32())              mowgli_strlcat(buf, " Int32", max);
	if(val->IsUint32())             mowgli_strlcat(buf, " Uint32", max);
	if(val->IsDate())               mowgli_strlcat(buf, " Date", max);
	if(val->IsArgumentsObject())    mowgli_strlcat(buf, " ArgumentsObject", max);
	if(val->IsBooleanObject())      mowgli_strlcat(buf, " BooleanObject", max);
	if(val->IsNumberObject())       mowgli_strlcat(buf, " NumberObject", max);
	if(val->IsStringObject())       mowgli_strlcat(buf, " StringObject", max);
	if(val->IsSymbolObject())       mowgli_strlcat(buf, " SymbolObject", max);
	if(val->IsNativeError())        mowgli_strlcat(buf, " NativeError", max);
	if(val->IsRegExp())             mowgli_strlcat(buf, " RegExp", max);
	if(val->IsGeneratorFunction())  mowgli_strlcat(buf, " GeneratorFunction", max);
	if(val->IsGeneratorObject())    mowgli_strlcat(buf, " GeneratorObject", max);
	if(val->IsPromise())            mowgli_strlcat(buf, " Promise", max);
	if(val->IsMap())                mowgli_strlcat(buf, " Map", max);
	if(val->IsSet())                mowgli_strlcat(buf, " Set", max);
	if(val->IsMapIterator())        mowgli_strlcat(buf, " MapIterator", max);
	if(val->IsSetIterator())        mowgli_strlcat(buf, " SetIterator", max);
	if(val->IsWeakMap())            mowgli_strlcat(buf, " WeakMap", max);
	if(val->IsWeakSet())            mowgli_strlcat(buf, " WeakSet", max);
	if(val->IsArrayBuffer())        mowgli_strlcat(buf, " ArrayBuffer", max);
	if(val->IsArrayBufferView())    mowgli_strlcat(buf, " ArrayBufferView", max);
	if(val->IsTypedArray())         mowgli_strlcat(buf, " TypedArray", max);
	if(val->IsUint8Array())         mowgli_strlcat(buf, " Uint8Array", max);
	if(val->IsUint8ClampedArray())  mowgli_strlcat(buf, " Uint8ClampedArray", max);
	if(val->IsInt8Array())          mowgli_strlcat(buf, " Int8Array", max);
	if(val->IsUint16Array())        mowgli_strlcat(buf, " Uint16Array", max);
	if(val->IsInt16Array())         mowgli_strlcat(buf, " Int16Array", max);
	if(val->IsUint32Array())        mowgli_strlcat(buf, " Uint32Array", max);
	if(val->IsInt32Array())         mowgli_strlcat(buf, " Int32Array", max);
	if(val->IsFloat32Array())       mowgli_strlcat(buf, " Float32Array", max);
	if(val->IsFloat64Array())       mowgli_strlcat(buf, " Float64Array", max);
	//if(val->IsFloat32x4())          mowgli_strlcat(buf, " Float32x4", max);
	if(val->IsDataView())           mowgli_strlcat(buf, " DataView", max);
	if(val->IsSharedArrayBuffer())  mowgli_strlcat(buf, " SharedArrayBuffer", max);
	if(val->IsProxy())              mowgli_strlcat(buf, " Proxy", max);

	return mowgli_strlcat(buf, " ]", max);
}


inline
ssize_t debug_object(char *const &buf,
                     const ssize_t &max,
                     v8::Local<v8::Object> obj,
                     v8::Local<v8::Context> context = {})
{
	if(!max)
		return 0;

	if(obj.IsEmpty())
		return mowgli_strlcpy(buf, "<EMPTY HANDLE>", max);

	auto creator_ctx(obj->CreationContext());
	const auto keys(obj->GetPropertyNames());
	const auto own_keys(obj->GetOwnPropertyNames());
	const size_t num_keys(keys->Length());
	const size_t num_own_keys(own_keys->Length());

	char attrs[128];
	attrs[0] = '\0';
	if(obj->IsCallable())                    mowgli_strlcat(attrs, " callable", sizeof(attrs));
	//if(obj->IsConstructor())                 mowgli_strlcat(attrs, " ctor", sizeof(attrs));
	if(obj->HasNamedLookupInterceptor())     mowgli_strlcat(attrs, " name-intercept", sizeof(attrs));
	if(obj->HasIndexedLookupInterceptor())   mowgli_strlcat(attrs, " idx-intercept", sizeof(attrs));

	char *ptr(buf);
	ssize_t rem(max);
	rem -= snprintf(ptr, rem,
	                "@[%-15p] id: %-12d creator: %-12d internal: %-2d keys: %-3zu own: %-3zu ctor: %-16s proto: %-16s attrs[%s ]",
	                (const void *)(*obj),
	                ident(obj),
	                ident(creator_ctx),
	                obj->InternalFieldCount(),
	                num_keys,
	                num_own_keys,
	                string(obj->GetConstructorName()),
	                test(context)? string(obj->ObjectProtoToString(context)) : "-",
	                attrs);

	if(rem <= 0)
		return max;

	ptr += max - rem;
	return ptr - buf;
}


template<class T>
size_t debug_pintercept(char *const &buf,
                        const ssize_t &max,
                        void *const &factory,
                        const char *const &operation,
                        const v8::Local<v8::Name> &name,
                        const v8::PropertyCallbackInfo<T> &arg)
{
	return snprintf(buf, max,
	               "factory(%16p): [%4s] %-20s in %-10d ( holder[%s] %10d in %-10d ) ( instance[%s] %10d in %-10d )",
	               factory,
	               operation,
	               test(name)? string(name) : "-",
	               in_ctx()? ident(ctx()) : 0,
	               string(holder(arg)->GetConstructorName()),
	               ident(holder(arg)),
	               ident(creator(holder(arg))),
	               string(instance(arg)->GetConstructorName()),
	               ident(instance(arg)),
	               ident(creator(instance(arg))));
}


template<class T>
size_t debug_fintercept(char *const &buf,
                        const ssize_t &max,
                        void *const &factory,
                        const char *const &operation,
                        const v8::Local<v8::Name> &name,
                        const v8::FunctionCallbackInfo<T> &arg)
{
	return snprintf(buf, max,
	               "factory(%16p): [%4s] %-20s in %-10d ( holder[%s] %10d in %-10d ) ( instance[%s] %10d in %-10d )",
	               factory,
	               operation,
	               test(name)? string(name) : "-",
	               in_ctx()? ident(ctx()) : 0,
	               string(holder(arg)->GetConstructorName()),
	               ident(holder(arg)),
	               ident(creator(holder(arg))),
	               string(instance(arg)->GetConstructorName()),
	               ident(instance(arg)),
	               ident(creator(instance(arg))));
}


template<class T>
size_t debug_wintercept(char *const &buf,
                        const ssize_t &max,
                        void *const &factory,
                        const char *const &operation,
                        const v8::Local<v8::Name> &name,
                        const v8::WeakCallbackInfo<T> &arg)
{
	return snprintf(buf, max,
	               "factory(%16p): [%4s] %-20s in %-10d parameter @%p",
	               factory,
	               operation,
	               test(name)? string(name) : "-",
	               in_ctx()? ident(ctx()) : 0,
	               arg.GetParameter());
}


template<class T>
void slog_intercept(void *const &factory,
                    const char *const &operation,
                    const v8::PropertyCallbackInfo<T> &arg,
                    const v8::Local<v8::Name> &name)
{
#ifdef DEBUG_INTERCEPTS
	slog_debug(debug_pintercept<T>, factory, operation, name, arg);
#endif
}


template<class T>
void slog_intercept(void *const &factory,
                    const char *const &operation,
                    const v8::FunctionCallbackInfo<T> &arg)
{
#ifdef DEBUG_INTERCEPTS
	slog_debug(debug_fintercept<T>, factory, operation, string("--------"), arg);
#endif
}


template<class T>
void slog_intercept(void *const &factory,
                    const char *const &operation,
                    const v8::WeakCallbackInfo<T> &arg,
                    const v8::Local<v8::Name> &name)
{
#ifdef DEBUG_INTERCEPTS
	slog_debug(debug_wintercept<T>, factory, operation, name, arg);
#endif
}


template<class T>
void slog_intercept(void *const &factory,
                    const char *const &operation,
                    const T &arg,
                    const uint32_t &idx)
{
#ifdef DEBUG_INTERCEPTS
	char name[32];
	snprintf(name, sizeof(name), "%d", idx);
	slog_intercept(factory, operation, arg, string(name));
#endif
}
