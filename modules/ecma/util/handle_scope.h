/*
 * Copyright (C) 2016 Jason Volk
 *
 */

struct handle_scope
{
	v8::HandleScope hs;

	handle_scope();
};


inline
handle_scope::handle_scope()
:hs(v8::Isolate::GetCurrent())
{
}
