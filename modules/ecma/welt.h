/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct welt
:factory
{
	// Child function and object templates
	std::map<std::string, welt *> children;
	std::map<std::string, function::factory *> functions;

	auto child_has(const std::string &n) const   { return children.count(n);                       }
	auto func_has(const std::string &n) const    { return functions.count(n);                      }
	auto &child(const std::string &n) const      { return *children.at(n);                         }
	auto &child(const std::string &n)            { return *children.at(n);                         }

	bool func_add(const std::string &name, function::factory *const &obj);
	bool func_del(const std::string &name);
	bool child_add(const std::string &path, welt *const &obj);
	bool child_del(const std::string &path);

	get_ret binpath_get(name_arg &, const get_arg &);
	qry_ret binpath_qry(name_arg &, const qry_arg &);
	get_ret child_new(const std::string &name, const get_arg &);
	get_ret func_new(const std::string &name, const get_arg &);

	// Generate new instances of a registered child/function if your child
	// factories are not registered in the containers here.
	template<class arg, class name> bool has_instance(const arg &, const name &) const;
	get_ret get_new(const std::string &name, const get_arg &);

	// These are inherited by default if you don't override them,
	// your object participates normally in the object tree.
	LS1::string help(const get_arg &arg);
	get_ret get(const get_arg &, name_arg &) override;
	qry_ret qry(const qry_arg &, name_arg &) override;
	enu_ret enu(const enu_arg &) override;

	welt(const size_t &internal_field_count = 0);
};


inline
welt::welt(const size_t &internal_field_count)
:factory{internal_field_count}
{
}


inline
welt::enu_ret
welt::enu(const enu_arg &arg)
{
	using namespace v8;

	auto ret(Array::New(isolate(), children.size() + functions.size()));
	size_t i(0);

	for(const auto &cp : children)
		::set(ret, i++, LS1::string(cp.first));

	for(const auto &fp : functions)
		::set(ret, i++, LS1::string(fp.first));

	return ret;
}


inline
welt::qry_ret
welt::qry(const qry_arg &arg,
          name_arg &name)
{
	return {};
}


inline
welt::get_ret
welt::get(const get_arg &arg,
          name_arg &name)
{
	if(has_instance(arg, name))
		return {};

	auto ret(get_new(string(name), arg));
	if(!ret) switch(hash(string(name)))
	{
		case hash("help"):     ret = help(arg);                break;
		default:               ret = binpath_get(name, arg);   break;
	}

	return ret;
}


inline
welt::get_ret welt::get_new(const std::string &name,
                            const get_arg &arg)
{
	using namespace v8;

	const auto obj(child_new(name, arg));
	if(test(obj))
		return obj;

	const auto func(func_new(name, arg));
	if(test(func))
		return func;

	return {};
}


template<class arg,
         class name>
bool welt::has_instance(const arg &a,
                        const name &n)
const
{
	const bool already(has(a, n));
	return already && !intercontext(a);
}


inline
welt::get_ret welt::child_new(const std::string &name,
                              const get_arg &arg)
{
	using namespace v8;

	const auto it(children.find(name));
	if(it == end(children))
		return {};

	auto &factory(*it->second);
	return factory(current(arg));
}


inline
welt::get_ret welt::func_new(const std::string &name,
                             const get_arg &arg)
{
	using namespace v8;

	const auto it(functions.find(name));
	if(it == end(functions))
		return {};

	auto &factory(*it->second);
	return factory(current(arg));
}


//TODO: environment
inline
welt::get_ret welt::binpath_get(name_arg &name,
                                const get_arg &arg)
{
	if(child_has("bin")) try
	{
		auto &bin(children.at("bin"));
		auto ret(bin->get(arg, name));
		if(test(ret))
			return ret;
	}
	catch(const error<ENOENT> &e) {}

	if(child_has("lib")) try
	{
		auto &lib(children.at("lib"));
		auto ret(lib->get(arg, name));
		if(test(ret))
			return ret;
	}
	catch(const error<ENOENT> &e) {}

	if(child_has("sys")) try
	{
		auto &sys(children.at("sys"));
		auto ret(sys->get(arg, name));
		if(test(ret))
			return ret;
	}
	catch(const error<ENOENT> &e) {}

	return {};
}


//TODO: environment
inline
welt::qry_ret welt::binpath_qry(name_arg &name,
                                const qry_arg &arg)
{
	if(child_has("bin"))
	{
		auto &bin(children.at("bin"));
		auto ret(bin->qry(arg, name));
		if(test(ret))
			return ret;
	}

	if(child_has("lib"))
	{
		auto &lib(children.at("lib"));
		auto ret(lib->qry(arg, name));
		if(test(ret))
			return ret;
	}

	if(child_has("sys"))
	{
		auto &sys(children.at("sys"));
		auto ret(sys->qry(arg, name));
		if(test(ret))
			return ret;
	}

	return {};
}


inline
bool welt::child_del(const std::string &path)
{
	const auto name(split(path, "."));
	if(name.second.empty())
		return children.erase(name.first);

	if(!child_has(name.first))
		throw std::runtime_error("Missing path dependency");

	auto &c(child(name.first));
	return c.child_del(name.second);
}


inline
bool welt::child_add(const std::string &path,
                     welt *const &obj)
{
	const auto name(split(path, "."));
	if(name.second.empty())
	{
		children.emplace(name.first, obj);
		return true;
	}

	if(!child_has(name.first))
		throw std::runtime_error("Missing path dependency");

	auto &c(child(name.first));
	return c.child_add(name.second, obj);
}


inline
bool welt::func_del(const std::string &path)
{
	const auto name(split(path, "."));
	if(name.second.empty())
		return functions.erase(name.first);

	if(!child_has(name.first))
		throw std::runtime_error("Missing path dependency");

	auto &c(child(name.first));
	return c.func_del(name.second);
}


bool welt::func_add(const std::string &path,
                    function::factory *const &obj)
{
	const auto name(split(path, "."));
	if(name.second.empty())
	{
		const auto iit(functions.emplace(name.first, obj));
		return iit.second;
	}

	if(!child_has(name.first))
		throw std::runtime_error("Missing path dependency");

	auto &c(child(name.first));
	return c.func_add(name.second, obj);
}


inline
LS1::string welt::help(const get_arg &arg)
{
	if(children.empty() && functions.empty())
		return "No help available.";

	std::stringstream ss;
	ss << "The following are available:";
	for(const auto &cp : children)
		ss << " \2" << cp.first << "\2,";

	for(const auto &fp : functions)
		ss << " \2" << fp.first << "\2,";

	return LS1::string{ss.str()};
}
