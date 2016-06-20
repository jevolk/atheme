/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct literal
{
	const char *name;
	const char *text;
	struct options options;
	ScriptCompiler::CachedData cache;

  public:
	virtual Local<UnboundScript> operator()();
	virtual Local<Script> operator()(Local<Context>);

	literal(const char *const &name     = "",
	        const char *const &text     = "",
	        const struct options &      = { 0, 0, 0, false, false, false });
};

literal operator ""_script(const char *const text, const size_t len);


inline
literal::literal(const char *const &name,
                 const char *const &text,
                 const struct options &options)
:name{name}
,text{text}
,options{options}
{
}


inline
Local<Script>
literal::operator()(Local<Context> ctx)
{
	return compile(ctx, string(text), string(name), options);
}


inline
literal operator ""_script(const char *const text, const size_t len)
{
	return { "<literal>", text };
}
