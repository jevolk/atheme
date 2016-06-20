/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

inline namespace interceptor
{
	// return type aliases to clean up your impl overloads
	using enu_ret = v8::Local<v8::Array>;
	using qry_ret = v8::Local<v8::Integer>;   // LS1::integer { v8:: None, ReadOnly, DontEnum, DontDelete }
	using del_ret = v8::Local<v8::Boolean>;
	using get_ret = v8::Local<v8::Value>;
	using set_ret = v8::Local<v8::Value>;
	using call_ret = v8::Local<v8::Value>;

	// argument type aliases to clean up your impl overloads
	using name_arg = v8::Local<v8::Name>;
	using set_val = v8::Local<v8::Value>;
	using enu_arg = v8::PropertyCallbackInfo<v8::Array>;
	using qry_arg = v8::PropertyCallbackInfo<v8::Integer>;
	using del_arg = v8::PropertyCallbackInfo<v8::Boolean>;
	using get_arg = v8::PropertyCallbackInfo<v8::Value>;
	using set_arg = v8::PropertyCallbackInfo<v8::Value>;
	using call_arg = v8::FunctionCallbackInfo<v8::Value>;
}

struct factory
:v8::Global<v8::ObjectTemplate>
{
	using enu_ret = interceptor::enu_ret;
	using qry_ret = interceptor::qry_ret;
	using del_ret = interceptor::del_ret;
	using get_ret = interceptor::get_ret;
	using set_ret = interceptor::set_ret;
	using call_ret = interceptor::call_ret;

	// argument type aliases to clean up your impl overloads
	using name_arg = interceptor::name_arg;
	using set_val = interceptor::set_val;
	using enu_arg = interceptor::enu_arg;
	using qry_arg = interceptor::qry_arg;
	using del_arg = interceptor::del_arg;
	using get_arg = interceptor::get_arg;
	using set_arg = interceptor::set_arg;
	using call_arg = interceptor::call_arg;

	// Implement these for indexed property interception
	virtual set_ret iset(const set_arg &, const uint32_t &idx, set_val &);
	virtual get_ret iget(const get_arg &, const uint32_t &idx);
	virtual del_ret idel(const del_arg &, const uint32_t &idx);
	virtual qry_ret iqry(const qry_arg &, const uint32_t &idx);
	virtual enu_ret ienu(const enu_arg &);

	// Impelement these for named property interception
	virtual set_ret set(const set_arg &, name_arg &, set_val &);
	virtual get_ret get(const get_arg &, name_arg &);
	virtual del_ret del(const del_arg &, name_arg &);
	virtual qry_ret qry(const qry_arg &, name_arg &);
	virtual enu_ret enu(const enu_arg &);

	virtual call_ret call(const call_arg &);
	virtual call_ret ctor(const call_arg &);
	virtual void dtor(const call_arg &);

	// Create a new instance of the templated object in-der-welt.
	v8::Local<v8::Object> operator()(const v8::Local<v8::Context> &ctx);

  private: // Internal callbacks from v8
	static void _iset(uint32_t i, v8::Local<v8::Value> val, const v8::PropertyCallbackInfo<v8::Value> &) noexcept;
	static void _iget(uint32_t i, const v8::PropertyCallbackInfo<v8::Value> &) noexcept;
	static void _idel(uint32_t i, const v8::PropertyCallbackInfo<v8::Boolean> &) noexcept;
	static void _iqry(uint32_t i, const v8::PropertyCallbackInfo<v8::Integer> &) noexcept;
	static void _ienu(const v8::PropertyCallbackInfo<v8::Array> &) noexcept;

	static void _set(v8::Local<v8::Name>, v8::Local<v8::Value> val, const v8::PropertyCallbackInfo<v8::Value> &) noexcept;
	static void _get(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value> &) noexcept;
	static void _del(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Boolean> &) noexcept;
	static void _qry(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Integer> &) noexcept;
	static void _enu(const v8::PropertyCallbackInfo<v8::Array> &) noexcept;

	static void _call(const v8::FunctionCallbackInfo<v8::Value> &) noexcept;
	static void _dtor(const v8::FunctionCallbackInfo<v8::Value> &) noexcept;

	void bind(v8::Local<v8::ObjectTemplate>);

  public:
	factory(v8::Local<v8::ObjectTemplate>);                 // Bind existing ObjectTemplate to this
	factory(const size_t &internal_field_count = 0);        // Creates a new ObjectTemplate and binds
	virtual ~factory() noexcept;
};


inline
factory::factory(const size_t &internal_field_count)
:v8::Global<v8::ObjectTemplate>{[this, &internal_field_count]
{
	using namespace v8;

	const isolate_scope isolate_scope;
	const handle_scope handle_scope;

	Local<ObjectTemplate> temple(ObjectTemplate::New(isolate()));
	reserve_fields(temple, internal_field_count);
	bind(temple);
	return global(temple);
}()}
{
}


inline
factory::factory(v8::Local<v8::ObjectTemplate> temple)
:v8::Global<v8::ObjectTemplate>{[this, &temple]
{
	bind(temple);
	return global(temple);
}()}
{
}


factory::~factory()
noexcept
{
}


inline
void factory::bind(v8::Local<v8::ObjectTemplate> temple)
{
	using namespace v8;

	auto self(External::New(isolate(), this));
	temple->SetCallAsFunctionHandler(_call, self);
	temple->SetHandler(IndexedPropertyHandlerConfiguration
	{
		_iget,
		_iset,
		_iqry,
		_idel,
		_ienu,
		self,
		PropertyHandlerFlags::kNone,
	});
	temple->SetHandler(NamedPropertyHandlerConfiguration
	{
		_get,
		_set,
		_qry,
		_del,
		_enu,
		self,
		PropertyHandlerFlags::kNone,
	});
}


inline
v8::Local<v8::Object>
factory::operator()(const v8::Local<v8::Context> &ctx)
{
	auto ret(maybe(local(*this)->NewInstance(ctx)));
	auto dtor(v8::Function::New(ctx, _dtor, LS1::external(this)));
	::set(ret, priv("dtor"), maybe(dtor));
	return ret;
}


inline
void factory::_dtor(const v8::FunctionCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "dtor", arg);
	that.dtor(arg);
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: dtor error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled dtor exception");
}


inline
void factory::_call(const v8::FunctionCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	if(arg.IsConstructCall())
	{
		slog_intercept(&that, "ctor", arg);
		return_value(arg, that.ctor(arg));
	} else {
		slog_intercept(&that, "call", arg);
		return_value(arg, that.call(arg));
	}
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: call(): real exception: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled call() exception");
}


inline
void factory::_enu(const v8::PropertyCallbackInfo<v8::Array> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "enu", arg, {});
	return_value(arg, that.enu(arg));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property enu error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled named property enu exception");
}


inline
void factory::_qry(v8::Local<v8::Name> prop,
                   const v8::PropertyCallbackInfo<v8::Integer> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "qry", arg, prop);
	return_value(arg, that.qry(arg, prop));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property qry error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled named property qry exception");
}


inline
void factory::_del(v8::Local<v8::Name> prop,
                   const v8::PropertyCallbackInfo<v8::Boolean> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "del", arg, prop);
	return_value(arg, that.del(arg, prop));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property del error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled named property del exception");
}


inline
void factory::_get(v8::Local<v8::Name> prop,
                   const v8::PropertyCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "get", arg, prop);
	return_value(arg, that.get(arg, prop));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property get error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled named property get exception");
}


inline
void factory::_set(v8::Local<v8::Name> prop,
                   v8::Local<v8::Value> val,
                   const v8::PropertyCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "set", arg, prop);
	return_value(arg, that.set(arg, prop, val));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property set error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled named property set exception");
}


inline
void factory::_ienu(const v8::PropertyCallbackInfo<v8::Array> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "ienu", arg, {});
	return_value(arg, that.ienu(arg));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property enu error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled index property enu exception");
}


inline
void factory::_iqry(uint32_t idx,
                    const v8::PropertyCallbackInfo<v8::Integer> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "iqry", arg, idx);
	return_value(arg, that.iqry(arg, idx));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property qry error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled index property qry exception");
}


inline
void factory::_idel(uint32_t idx,
                    const v8::PropertyCallbackInfo<v8::Boolean> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "idel", arg, idx);
	return_value(arg, that.idel(arg, idx));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property del error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled index property del exception");
}


inline
void factory::_iget(uint32_t idx,
                    const v8::PropertyCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "iget", arg, idx);
	return_value(arg, that.iget(arg, idx));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property iget error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled index property get exception");
}


inline
void factory::_iset(uint32_t idx,
                    v8::Local<v8::Value> val,
                    const v8::PropertyCallbackInfo<v8::Value> &arg)
noexcept try
{
	if(!have_our_ctx())
		return;

	auto &that(privdata<factory>(arg));
	slog_intercept(&that, "iset", arg, idx);
	return_value(arg, that.iset(arg, idx, val));
}
catch(exception &e)
{
	auto &zeug(zeug::zeug::get(isolate(arg)));
	zeug.exception(e);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "factory: property iset error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "factory: Unhandled index property set exception");
}


inline
void
factory::dtor(const call_arg &arg)
{
}


inline
call_ret
factory::ctor(const call_arg &arg)
{
	return {};
}


inline
factory::call_ret
factory::call(const call_arg &arg)
{
	return {};
}


inline
factory::enu_ret
factory::enu(const enu_arg &arg)
{
	return {};
}


inline
factory::qry_ret
factory::qry(const qry_arg &arg,
             name_arg &name)
{
	return {};
}


inline
factory::del_ret
factory::del(const del_arg &arg,
             name_arg &name)
{
	return {};
}


inline
factory::get_ret
factory::get(const get_arg &arg,
             name_arg &name)
{
	return {};
}


inline
factory::set_ret
factory::set(const set_arg &arg,
             name_arg &name,
             set_val &val)
{
	return {};
}


inline
factory::enu_ret
factory::ienu(const enu_arg &arg)
{
	return {};
}


inline
factory::qry_ret
factory::iqry(const qry_arg &arg,
              const uint32_t &idx)
{
	return {};
}


inline
factory::del_ret
factory::idel(const del_arg &arg,
              const uint32_t &idx)
{
	return {};
}


inline
factory::get_ret
factory::iget(const get_arg &arg,
              const uint32_t &idx)
{
	return {};
}


inline
factory::set_ret
factory::iset(const set_arg &arg,
              const uint32_t &idx,
              set_val &val)
{
	return {};
}
