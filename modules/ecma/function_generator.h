/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct generator
:literal
{
	Local<Function> operator()(Local<Context>) override;

	using literal::literal;
};

generator operator ""_generator(const char *const text, const size_t len);


inline
Local<Function>
generator::operator()(Local<Context> ctx)
{
	std::vector<Local<Object>> extensions;
	std::vector<LS1::string> protv(begin(proto), end(proto));
	return compile_generator(ctx, string(text), protv, string(name), options, extensions);
}


inline
generator operator ""_generator(const char *const text, const size_t len)
{
	return { "<generator>", {}, text };
}
