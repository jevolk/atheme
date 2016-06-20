/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct home
:welt
{
}
static home;


DECLARE_MODULE_V1
(
	"ecma/home",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("home", &home);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("home");
	},
	PACKAGE_STRING,
	"jzk"
);
