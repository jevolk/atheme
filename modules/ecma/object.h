/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

template<class T>
struct object
:welt
{
	virtual set_ret iset(T &, const set_arg &, const uint32_t &idx, set_val &);
	virtual get_ret iget(T &, const get_arg &, const uint32_t &idx);
	virtual del_ret idel(T &, const del_arg &, const uint32_t &idx);
	virtual qry_ret iqry(T &, const qry_arg &, const uint32_t &idx);
	virtual enu_ret ienu(T &, const enu_arg &);

	virtual set_ret set(T &, const set_arg &, name_arg &, set_val &);
	virtual get_ret get(T &, const get_arg &, name_arg &);
	virtual del_ret del(T &, const del_arg &, name_arg &);
	virtual qry_ret qry(T &, const qry_arg &, name_arg &);
	virtual enu_ret enu(T &, const enu_arg &);

	virtual call_ret call(T &, const call_arg &);
	virtual void dtor(T &, const call_arg &);
	virtual T *init(const call_arg &);

  private:
	template<class arg> const T *pointer(const arg &) const;
	template<class arg> T *pointer(const arg &);

	set_ret iset(const set_arg &, const uint32_t &idx, set_val &) final override;
	get_ret iget(const get_arg &, const uint32_t &idx) final override;
	del_ret idel(const del_arg &, const uint32_t &idx) final override;
	qry_ret iqry(const qry_arg &, const uint32_t &idx) final override;
	enu_ret ienu(const enu_arg &) final override;

	set_ret set(const set_arg &, name_arg &, set_val &) final override;
	get_ret get(const get_arg &, name_arg &) final override;
	del_ret del(const del_arg &, name_arg &) final override;
	qry_ret qry(const qry_arg &, name_arg &) final override;
	enu_ret enu(const enu_arg &) final override;

	call_ret call(const call_arg &) final override;
	call_ret ctor(const call_arg &) final override;
	void dtor(const call_arg &) final override;

  public:
	v8::Local<v8::Object> operator()(const v8::Local<v8::Context> &, v8::Local<v8::Object> &, const function::argv & = {});
	v8::Local<v8::Object> operator()(const v8::Local<v8::Context> &, const function::argv & = {});
	v8::Local<v8::Object> operator()(const v8::Local<v8::Context> &, T *const &ptr);
	v8::Local<v8::Object> operator()(T *const &ptr);

	object() = default;
};


template<class T>
v8::Local<v8::Object>
object<T>::operator()(T *const &pointer)
{
	return operator()(ctx(), pointer);
}


template<class T>
v8::Local<v8::Object>
object<T>::operator()(const v8::Local<v8::Context> &ctx,
                      T *const &pointer)
{
	auto ret(welt::operator()(ctx));
	::set(ret, priv("pointer"), LS1::external(pointer));
	return ret;
}


template<class T>
v8::Local<v8::Object>
object<T>::operator()(const v8::Local<v8::Context> &ctx,
                      const function::argv &argv)
{
	auto obj(welt::operator()(ctx));
	return operator()(ctx, obj, argv);
}


template<class T>
v8::Local<v8::Object>
object<T>::operator()(const v8::Local<v8::Context> &ctx,
                      v8::Local<v8::Object> &obj,
                      const function::argv &argv)
{
	auto ret(::ctor(ctx, obj, argv));
	return mustbe<v8::Object>(ret);
}


template<class T>
void
object<T>::dtor(const call_arg &arg)
{
	const auto pointer(this->pointer(arg));
	if(pointer)
		dtor(*pointer, arg);
}


template<class T>
typename object<T>::call_ret
object<T>::ctor(const call_arg &arg)
{
	T *const pointer(init(arg));
	::set(arg, priv("pointer"), LS1::external(pointer));
	return instance(arg);
}


template<class T>
typename object<T>::call_ret
object<T>::call(const call_arg &arg)
{
	return call(*pointer(arg), arg);
}


template<class T>
typename object<T>::enu_ret
object<T>::enu(const enu_arg &arg)
{
	return enu(*pointer(arg), arg);
}


template<class T>
typename object<T>::qry_ret
object<T>::qry(const qry_arg &arg,
               name_arg &name)
{
	return qry(*pointer(arg), arg, name);
}


template<class T>
typename object<T>::del_ret
object<T>::del(const del_arg &arg,
               name_arg &name)
{
	return del(*pointer(arg), arg, name);
}


template<class T>
typename object<T>::get_ret
object<T>::get(const get_arg &arg,
               name_arg &name)
{
	return get(*pointer(arg), arg, name);
}


template<class T>
typename object<T>::set_ret
object<T>::set(const set_arg &arg,
               name_arg &name,
               set_val &val)
{
	return set(*pointer(arg), arg, name, val);
}


template<class T>
typename object<T>::enu_ret
object<T>::ienu(const enu_arg &arg)
{
	return ienu(*pointer(arg), arg);
}


template<class T>
typename object<T>::qry_ret
object<T>::iqry(const qry_arg &arg,
                const uint32_t &idx)
{
	return iqry(*pointer(arg), arg, idx);
}


template<class T>
typename object<T>::del_ret
object<T>::idel(const del_arg &arg,
                const uint32_t &idx)
{
	return idel(*pointer(arg), arg, idx);
}


template<class T>
typename object<T>::get_ret
object<T>::iget(const get_arg &arg,
                const uint32_t &idx)
{
	return iget(*pointer(arg), arg, idx);
}


template<class T>
typename object<T>::set_ret
object<T>::iset(const set_arg &arg,
                const uint32_t &idx,
                set_val &val)
{
	return iset(*pointer(arg), arg, idx, val);
}


template<class T>
template<class arg>
T *
object<T>::pointer(const arg &a)
{
	return ::get<T *>(a, priv("pointer"));
}


template<class T>
template<class arg>
const T *
object<T>::pointer(const arg &a)
const
{
	return ::get<const T *>(a, priv("pointer"));
}


template<class T>
void
object<T>::dtor(T &,
                const call_arg &)
{
}


template<class T>
T *
object<T>::init(const call_arg &arg)
{
	return nullptr;
}


template<class T>
typename object<T>::call_ret
object<T>::call(T& t,
                const call_arg &arg)
{
	return {};
}


template<class T>
typename object<T>::enu_ret
object<T>::enu(T &t,
               const enu_arg &arg)
{
	return {};
}


template<class T>
typename object<T>::qry_ret
object<T>::qry(T &t,
               const qry_arg &arg,
               name_arg &name)
{
	return {};
}


template<class T>
typename object<T>::del_ret
object<T>::del(T &t,
               const del_arg &arg,
               name_arg &name)
{
	return {};
}


template<class T>
typename object<T>::get_ret
object<T>::get(T &t,
               const get_arg &arg,
               name_arg &name)
{
	return {};
}


template<class T>
typename object<T>::set_ret
object<T>::set(T &t,
               const set_arg &arg,
               name_arg &name,
               set_val &val)
{
	return {};
}


template<class T>
typename object<T>::enu_ret
object<T>::ienu(T &t,
                const enu_arg &arg)
{
	return {};
}


template<class T>
typename object<T>::qry_ret
object<T>::iqry(T &t,
                const qry_arg &arg,
                const uint32_t &idx)
{
	return {};
}


template<class T>
typename object<T>::del_ret
object<T>::idel(T &t,
                const del_arg &arg,
                const uint32_t &idx)
{
	return {};
}


template<class T>
typename object<T>::get_ret
object<T>::iget(T &t,
                const get_arg &arg,
                const uint32_t &idx)
{
	return {};
}


template<class T>
typename object<T>::set_ret
object<T>::iset(T &t,
                const set_arg &arg,
                const uint32_t &idx,
                set_val &val)
{
	return {};
}
