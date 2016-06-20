/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

using script::origin;
using script::source;
using script::options;
using argv = std::vector<Local<Value>>;
using prototype = std::vector<LS1::string>;
using extensions = std::vector<Local<Object>>;

template<class T> argv make_argv(const v8::FunctionCallbackInfo<T> &arg);
Local<String> generator_wrapper(const Local<String> &code);

argv extract(const Local<Context> &, const Local<Function> &);
void bind(const Local<Context> &, const Local<Function> &, const argv &);

Local<Function> compile(const Local<Context> &, source &, prototype &, extensions &);
Local<Function> compile(const Local<Context> &, const Local<String> &src, const prototype & = {}, const Local<String> &name = {}, const options &opts = {}, const extensions & ={});
Local<Function> compile_generator(const Local<Context> &, const Local<String> &src, const prototype & = {}, const Local<String> &name = {}, const options &opts = {}, const extensions & = {});


inline
Local<Function>
compile_generator(const Local<Context> &ctx,
                  const Local<String> &text,
                  const prototype &prot,
                  const Local<String> &name,
                  const options &opts,
                  const extensions &ext)
{
	const auto full(generator_wrapper(text));
	return compile(ctx, full, prot, name, opts, ext);
}


inline
Local<Function>
compile(const Local<Context> &ctx,
        const Local<String> &text,
        const prototype &prot,
        const Local<String> &name,
        const options &opts,
        const extensions &ext)
{
	source src
	{
		text,
		script::generate_origin(name, opts),
		nullptr, //cache
	};

	prototype p(prot);
	extensions e(ext);
	return compile(ctx, src, p, e);
}


inline
Local<Function>
compile(const Local<Context> &c,
        source &s,
        prototype &p,
        extensions &e)
{
	const context_scope context_scope(c);
	return maybe<syntax_error>([&]
	{
		return ScriptCompiler::CompileFunctionInContext(c, &s, p.size(), p.data(), e.size(), e.data());
	});
}


inline
void bind(const Local<Context> &ctx,
          const Local<Function> &function,
          const argv &a)
{
	for(size_t i(0); i < a.size(); ++i)
		::set(ctx, function, i, a[i]);
}


inline
argv extract(const Local<Context> &ctx,
             const Local<Function> &function)
{
	argv ret;
	ret.reserve(8);
	for(size_t i(0); has(ctx, function, i) && i < 8; ++i)
		ret.emplace_back(get(ctx, function, i));

	return ret;
}


inline
Local<String>
generator_wrapper(const Local<String> &code)
{
	static const char *const head
	{R"(

		return function *()
		{

	)"};

	static const char *const tail
	{R"(

		}();

	)"};

	auto full(v8::String::Concat(LS1::string(head), code));
	full = v8::String::Concat(full, LS1::string(tail));
	return full;
}


template<class T>
argv make_argv(const v8::FunctionCallbackInfo<T> &arg)
{
	argv ret(arg.Length());
	for(size_t i(0); i < ret.size(); ++i)
		ret[i] = arg[i];

	return ret;
}
