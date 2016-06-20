/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

using origin = ScriptOrigin;
using source = ScriptCompiler::Source;

struct options
{
	uint32_t line                  { 0                                     };
	uint32_t column                { 0                                     };
	uint32_t script_id             { 0                                     };
	bool shared_cross_origin       { false                                 };
	bool embedder_debug            { false                                 };
	bool opaque                    { false                                 };
};

origin generate_origin(const Local<String> &name, const options &);
Local<String> generator_wrapper(const Local<String> &code);

Local<UnboundScript> compile(source &);
Local<UnboundScript> compile(Local<String> code, Local<String> name = {}, const options & = {});
Local<UnboundScript> compile_generator(Local<String> code, Local<String> name = {}, const options &opts = {});

Local<Script> compile(Local<Context> ctx, source &);
Local<Script> compile(Local<Context> ctx, Local<String> code, Local<String> name = {}, const options & = {});
Local<Script> compile_generator(Local<Context> ctx, Local<String> code, Local<String> name = {}, const options &opts = {});


inline
Local<Script>
compile_generator(const Local<Context> &ctx,
                  const Local<String> &text,
                  const Local<String> &name,
                  const options &opts)
{
	const auto full(generator_wrapper(text));
	return compile(ctx, full, name, opts);
}


inline
Local<Script>
compile(Local<Context> ctx,
        Local<String> code,
        Local<String> name,
        const options &opts)
{
	source src
	{
		code,
		generate_origin(name, opts),
		nullptr, //cache
	};

	return compile(ctx, src);
}


inline
Local<Script>
compile(Local<Context> ctx,
        source &src)
{
	const context_scope context_scope(ctx);
	return maybe<syntax_error>([&ctx, &src]
	{
		return ScriptCompiler::Compile(ctx, &src);
	});
}


inline
Local<UnboundScript>
compile_generator(const Local<String> &text,
                  const Local<String> &name,
                  const options &opts)
{
	const auto full(generator_wrapper(text));
	return compile(full, name, opts);
}


inline
Local<UnboundScript>
compile(Local<String> code,
        Local<String> name,
        const options &opts)
{
	source src
	{
		code,
		generate_origin(name, opts),
		nullptr, //cache
	};

	return compile(src);
}


inline
Local<UnboundScript>
compile(source &src)
{
	return maybe<syntax_error>([&src]
	{
		return ScriptCompiler::CompileUnboundScript(isolate(), &src);
	});
}


inline
Local<String>
generator_wrapper(const Local<String> &code)
{
	static const char *const head
	{R"(

		function *main(argv)
		{

	)"};

	static const char *const tail
	{R"(

		}
		main(argv);

	)"};

	auto full(v8::String::Concat(LS1::string(head), code));
	full = v8::String::Concat(full, LS1::string(tail));
	return full;
}


inline
origin generate_origin(const Local<String> &name,
                       const struct options &options)
{
	return
	{
		name,
		LS1::integer(options.line),
		LS1::integer(options.column),
		LS1::boolean(options.shared_cross_origin),
		LS1::integer(options.script_id),
		LS1::boolean(options.embedder_debug),
		{},
		LS1::boolean(options.opaque)
	};
}
