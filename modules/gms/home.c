/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_home(sourceinfo_t *si, int parc, char *parv[])
{
	if(!parc)
	{
		command_success_nodata(si, _("****** \2" GMS_TITLE "\2 ******"));


		command_success_nodata(si, _("***** \2End of Home\2 *****"));
		return;
	}
	else help_display_as_subcmd(si, si->service, NULL, parv[0], si->service->commands);
}


command_t cmd =
{
	"HOME",
	N_(N_("Display information and events for my account.")),
	AC_AUTHENTICATED,
	0,
	gms_home,
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
	GMS_MODULE"/home",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
