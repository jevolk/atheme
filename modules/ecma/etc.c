/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


struct etc
:welt
{
}
static etc;


DECLARE_MODULE_V1
(
	"ecma/etc",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		ecma->add("etc", &etc);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("etc");
	},
	PACKAGE_STRING,
	"jzk"
);
