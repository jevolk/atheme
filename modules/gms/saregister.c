/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_saregister(sourceinfo_t *si, int parc, char *parv[])
{
	const char *const name = parv[0];
	if(!gms_create_group(gms, name))
	{
		command_fail(si, fault_badparams, "%s", gmserr);
		return;
	}

	command_success_nodata(si, "Successfully registered \2%s\2", name);
}


command_t cmd =
{
	"SAREGISTER",
	N_(N_("Services admin manual group registration.")),
	AC_SRA,
	1,
	gms_saregister,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	service_bind_command(myservice, &cmd);
}


void module_fini(module_unload_intent_t intent)
{
	service_unbind_command(myservice, &cmd);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	GMS_MODULE"/saregister",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
