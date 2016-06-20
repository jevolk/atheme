/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

template<class T>
struct member
{
	using set_ret = factory::set_ret;
	using set_arg = factory::set_arg;
	using set_val = factory::set_val;
	using get_ret = factory::get_ret;
	using get_arg = factory::get_arg;
	using del_ret = factory::del_ret;
	using del_arg = factory::del_arg;
	using qry_ret = factory::qry_ret;
	using qry_arg = factory::qry_arg;

	std::function<get_ret (T &, const get_arg &)> get;
	std::function<set_ret (T &, const set_arg &, set_val &)> set;
	std::function<del_ret (T &, const del_arg &)> del;
	std::function<qry_ret (T &, const qry_arg &)> qry;
};


template<class T>
struct structure
:object<T>
{
	using pair = std::pair<const char *, member<T>>;

	std::map<std::string, member<T>> map;
	bool throw_not_found = true;
	bool throw_unsupported = true;
	bool qry_on_found = true;

	set_ret set(T &, const set_arg &, name_arg &, set_val &) override;
	del_ret del(T &, const del_arg &, name_arg &) override;
	get_ret get(T &, const get_arg &, name_arg &) override;
	qry_ret qry(T &, const qry_arg &, name_arg &) override;
	enu_ret enu(T &, const enu_arg &) override;

	structure(std::initializer_list<pair> init = {});
};


template<class T>
structure<T>::structure(std::initializer_list<pair> init)
{
	std::for_each(begin(init), end(init), [this]
	(const auto &p)
	{
		map.insert(p);
	});
}


template<class T>
interceptor::enu_ret
structure<T>::enu(T &ref,
                  const enu_arg &arg)
{
	auto parent(welt::enu(arg));
	LS1::array ret(parent->Length() + map.size());

	size_t i(0);
	for(; i < parent->Length(); ++i)
		::set(ret, i, ::get(parent, i));

	std::for_each(begin(map), end(map), [&i, &ret]
	(const auto &pit)
	{
		::set(ret, i++, LS1::string(pit.first));
	});

	return ret;
}


template<class T>
interceptor::qry_ret
structure<T>::qry(T &ref,
                  const qry_arg &arg,
                  name_arg &name)
try
{
	const auto &member(map.at(string(name)));
	if(!member.qry && qry_on_found)
		return LS1::integer(v8::None);

	if(!member.qry)
		return {};

	return member.qry(ref, arg);
}
catch(const std::out_of_range &e)
{
	if(throw_not_found)
		throw error<ENOENT>(string(name));

	return {};
}


template<class T>
interceptor::del_ret
structure<T>::del(T &ref,
                  const del_arg &arg,
                  name_arg &name)
try
{
	auto &member(map.at(string(name)));
	if(!member.del && throw_unsupported)
		throw error<ENOTSUP>("delete unsupported");

	if(!member.del)
		return {};

	return member.del(ref, arg);
}
catch(const std::out_of_range &e)
{
	if(throw_not_found)
		throw error<ENOENT>(string(name));

	return {};
}


template<class T>
interceptor::get_ret
structure<T>::get(T &ref,
                  const get_arg &arg,
                  name_arg &name)
try
{
	auto &member(map.at(string(name)));
	if(!member.get && throw_unsupported)
		throw error<ENOTSUP>("reading unsupported");

	if(!member.get)
		return {};

	return member.get(ref, arg);
}
catch(const std::out_of_range &e)
{
	if(throw_not_found)
		throw error<ENOENT>(string(name));

	return {};
}


template<class T>
interceptor::set_ret
structure<T>::set(T &ref,
                  const set_arg &arg,
                  name_arg &name,
                  set_val &val)
try
{
	auto &member(map.at(string(name)));
	if(!member.set && throw_unsupported)
		throw error<ENOTSUP>("writing unsupported");

	if(!member.set)
		return {};

	return member.set(ref, arg, val);
}
catch(const std::out_of_range &e)
{
	if(throw_not_found)
		throw error<ENOENT>(string(name));

	return {};
}
