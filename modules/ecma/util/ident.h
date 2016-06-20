/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
auto ident(const v8::Local<v8::Object> &obj)
{
	return obj->GetIdentityHash();
}


inline
auto ident(const v8::Local<v8::Context> &ctx)
{
	return ident(ctx->Global());
}


inline
auto ident(context_scope &context_scope)
{
	return ident(context_scope.context->Global());
}
