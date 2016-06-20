/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

module_t *mymodule;


static
void module_abort()
{
	module_unload(mymodule, MODULE_UNLOAD_INTENT_PERM);
}


static
void module_abort_all()
{
	if(ecma)
		module_unload(ecma->module, MODULE_UNLOAD_INTENT_PERM);
}


template<class T = void>
T **link_symbol(const char *const &them,
                const char *const &name)
{
	void *ptr;
	if((ptr = module_locate_symbol(them, name)))
		return reinterpret_cast<T **>(ptr);

	if(!module_request(them))
		throw std::runtime_error("failed to find module");

	if(!(ptr = module_locate_symbol(them, name)))
		throw std::runtime_error("failed to find symbol in module");

	return reinterpret_cast<T **>(ptr);
}


static
void module_register(module_t *const m)
{
	mymodule = m;
	mowgli_node_add(ecma->module, mowgli_node_create(), &m->deplist);
	mowgli_node_add(m, mowgli_node_create(), &ecma->module->dephost);
	slog(LG_DEBUG, "ECMA: Registered submodule %s.", m->name);
}


__attribute__((constructor))
static
void module_init_common()
noexcept try
{
	ecma = *link_symbol<ECMA>("ecma/ecma", "ecma");
	isolate_extern = link_symbol<v8::Isolate>("ecma/ecma", "extern_isolate");
	context_scope_current = link_symbol<context_scope>("ecma/ecma", "extern_context_scope");

	if(ecma)
		myservice = ecma->svc.get();
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "module_init_common() (constructor): %s", e.what());
	throw;
}


__attribute__((destructor))
static
void module_fini_common()
noexcept try
{
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "module_fini_common() (destructor): %s", e.what());
	throw;
}
