/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_invite(sourceinfo_t *si, int parc, char *parv[])
{

}


command_t cmd =
{
	"INVITE",
	N_(N_("Invite a group member.")),
	AC_AUTHENTICATED,
	2,
	gms_invite,
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
	GMS_MODULE"/invite",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
