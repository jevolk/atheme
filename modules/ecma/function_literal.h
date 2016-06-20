/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct literal
{
	using prototype = std::initializer_list<const char *>;

	const char *name;
	const char *text;
	prototype proto;
	struct options options;
	ScriptCompiler::CachedData cache;

  public:
	virtual Local<Function> operator()(Local<Context>);

	literal(const char *const &name     = "",
	        const prototype &prototype  = {},
	        const char *const &text     = "",
	        const struct options &      = { 0, 0, 0, false, false, false });
};

literal operator ""_function(const char *const text, const size_t len);


inline
literal::literal(const char *const &name,
                 const prototype &prototype,
                 const char *const &text,
                 const struct options &options)
:name{name}
,text{text}
,proto{prototype}
,options{options}
{
}


inline
Local<Function>
literal::operator()(Local<Context> ctx)
{
	source src
	{
		LS1::string(text),
		generate_origin(string(name), options),
		nullptr, //&cache
	};

	std::vector<Local<Object>> extensions;
	std::vector<LS1::string> protv(begin(proto), end(proto));
	return compile(ctx, src, protv, extensions);
}


inline
literal operator ""_function(const char *const text, const size_t len)
{
	return { "<literal>", {}, text };
}
