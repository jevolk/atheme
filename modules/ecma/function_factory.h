/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct factory
:Global<FunctionTemplate>
{
	using call_ret = ::factory::call_ret;
	using call_arg = ::factory::call_arg;

	struct options
	{
		const char *name               { ""                                    };
		int nargs                      { 0                                     };
		bool accept_any                { true                                  };
		bool proto_remove              { false                                 };
		bool proto_hidden              { false                                 };
		bool proto_read_only           { false                                 };
	};

	// Override to provide functionality
	virtual call_ret call(const call_arg &) = 0;

	// Create new instance of this function in a context.
	Local<Function> operator()(Local<Context>);

	factory(const options & = {"<unnamed>", 0, true, false, false, false});
	virtual ~factory() noexcept;

  private:
	static void _call(const call_arg &) noexcept;
};


inline
factory::factory(const options &options)
:Global<FunctionTemplate>{[this, &options]
{
	using namespace v8;

	const isolate_scope isolate_scope;
	const handle_scope handle_scope;
	auto self(External::New(isolate(), this));
	auto ret(FunctionTemplate::New(isolate(), _call, self));
	ret->SetClassName(LS1::string(options.name));
	ret->SetLength(options.nargs);
	ret->SetAcceptAnyReceiver(options.accept_any);
	ret->SetHiddenPrototype(options.proto_hidden);

	if(options.proto_read_only)
		ret->ReadOnlyPrototype();

	if(options.proto_remove)
		ret->RemovePrototype();

	return global(ret);
}()}
{
}


inline
factory::~factory()
noexcept
{
}


Local<Function>
factory::operator()(Local<Context> c)
{
	const context_scope context_scope(c);
	return maybe(local(*this)->GetFunction(c), "internal error: function instantiation");
}


inline
void factory::_call(const call_arg &arg)
noexcept try
{
	auto &that(privdata<factory>(arg));
	arg.GetReturnValue().Set(that.call(arg));
}
catch(exception &e)
{
	isolate(arg)->ThrowException(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Function call error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "Unhandled function call exception.");
}
