/*
 * Copyright (C) 2016 Jason Volk
 *
 */


struct context_scope
{
	struct exit
	{
		exit();
		~exit();
	};

	context_scope *theirs;
	v8::Local<v8::Context> context;

	context_scope(v8::Local<v8::Context> context);
	~context_scope();
};


extern "C"
{
	__thread context_scope **context_scope_current;
}


inline
context_scope::context_scope(v8::Local<v8::Context> context)
:theirs{context_scope_current? *context_scope_current : nullptr}
,context{context}
{
	context->Enter();
	if(likely(bool(context_scope_current)))
		*context_scope_current = this;
}


inline
context_scope::~context_scope()
{
	context->Exit();
	if(likely(bool(context_scope_current)))
		*context_scope_current = theirs;
}


inline
context_scope::exit::exit()
{
	if(likely(context_scope_current && *context_scope_current))
		(*context_scope_current)->context->Exit();
}


inline
context_scope::exit::~exit()
{
	if(likely(context_scope_current && *context_scope_current))
		(*context_scope_current)->context->Enter();
}
