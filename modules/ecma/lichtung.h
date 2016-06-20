/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct lichtung
{
	enum field
	{
		THIS,
		DASEIN,
		_NUM_
	};

	v8::Global<v8::Context> context;

	operator v8::Local<v8::Context>() const      { return local(context);                          }
	operator v8::Local<v8::Context>()            { return local(context);                          }
	auto ident() const                           { return ::ident(local(context));                 }
	auto global() const                          { return local(context)->Global();                }
	auto global()                                { return local(context)->Global();                }
	void detach()                                { local(context)->DetachGlobal();                 }

	template<class T = v8::Value> auto get(const field &) const -> v8::Local<T>;
	template<class T = void> T *get_ptr(const field &) const;
	void set(const field &, v8::Local<v8::Value> data);
	void set(const field &, void *const &ptr);

  private:
	using field_array = std::array<v8::Local<v8::Value>, num_of<field>()>;
	field_array get_field_array();
	void set_field_array(field_array &field_array);
	void init_fields();

  public:
	lichtung(welt &, v8::Local<v8::Object> global = {});

	// Extract an instance from upstream types:
	static lichtung &get(v8::Local<v8::Context> ctx);
	template<class T> static lichtung &get(const v8::PropertyCallbackInfo<T> &);
	template<class T> static lichtung &get(const v8::FunctionCallbackInfo<T> &);
};


inline
lichtung::lichtung(welt &welt,
                   v8::Local<v8::Object> global)
:context{[&]
{
	using namespace v8;

	auto ret(Context::New(isolate(), nullptr, local(welt), global));
	assert(test(ret));

	ret->SetSecurityToken(LS1::integer(1337));
	return Global<Context>{isolate(), ret};
}()}
{
	init_fields();
	set(field::THIS, this);
}


inline
void lichtung::init_fields()
{
	for(size_t i(0); i < num_of<field>(); i++)
		set(field(i), nullptr);
}


inline
lichtung::field_array
lichtung::get_field_array()
{
	field_array ret;
	for(size_t i(0); i < ret.size(); i++)
		ret[i] = get(field(i));

	return ret;
}


inline
void lichtung::set_field_array(field_array &fa)
{
	for(size_t i(0); i < fa.size(); i++)
		set(field(i), fa[i]);
}


inline
void lichtung::set(const field &f,
                   void *const &ptr)
{
	const context_scope context_scope(*this);
	set(f, v8::External::New(isolate(), ptr));
}


inline
void lichtung::set(const field &f,
                   v8::Local<v8::Value> data)
{
	local(context)->SetEmbedderData(int(f), data);
}


template<class T>
T *lichtung::get_ptr(const field &f)
const
{
	auto data(get<v8::External>(f));
	return reinterpret_cast<T *>(data->Value());
}


template<class T>
auto lichtung::get(const field &f)
const
-> v8::Local<T>
{
	return local(context)->GetEmbedderData(int(f)).As<v8::External>();
}


template<class T>
lichtung &lichtung::get(const v8::FunctionCallbackInfo<T> &arg)
{
	return get(current(arg));
}


template<class T>
lichtung &lichtung::get(const v8::PropertyCallbackInfo<T> &arg)
{
	return get(current(arg));
}


inline
lichtung &lichtung::get(v8::Local<v8::Context> context)
{
	const auto field(lichtung::field::THIS);
	auto ext(context->GetEmbedderData(int(field)).As<v8::External>());
	return *static_cast<lichtung *>(ext->Value());
}
