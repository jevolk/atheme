/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"

v8::Isolate *extern_isolate;
__thread context_scope *extern_context_scope;


ECMA::ECMA(module_t *const &m)
:module{m}
,svc
{
	[]() -> service_t *
	{
		const auto ret(service_add("irc", nullptr));
		myservice = ret;
		return ret;
	}(),
	[](service_t *const svc)
	{
		service_delete(svc);
	}
}
,mitsein{zeug, ereignis}
{
}


ECMA::~ECMA()
{
}


static
void init(module_t *const m)
noexcept
{
	isolate_extern = &extern_isolate;
	context_scope_current = &extern_context_scope;
	ecma = new ECMA{m};
}

static
void fini(module_unload_intent_t)
noexcept
{
	delete ecma;
}

DECLARE_MODULE_V1
(
	"ecma/ecma",
	MODULE_UNLOAD_CAPABILITY_OK,
	init,
	fini,
	PACKAGE_STRING,
	"jzk"
);
