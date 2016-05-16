/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "pgsql.h"


void pgsql_help(sourceinfo_t *si, int parc, char *parv[])
{
	//spqf->helpcmd.first = parc;
	//spqf->helpcmd.second = parv;

	if(!parc)
	{
		command_success_nodata(si, _("****** \2 PostgreSQL \2 ******"));

		command_success_nodata(si, _("For more information on a command, type:"));
		command_success_nodata(si, "\2/%s%s help <command>\2", (ircd->uses_rcommand == false) ? "msg " : "", si->service->nick);
		command_success_nodata(si, " ");

		command_help(si, si->service->commands);

		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}
	else help_display_as_subcmd(si, si->service, NULL, parv[0], si->service->commands);
}


command_t cmd =
{
	"HELP",
	N_(N_("Displays contextual help information.")),
	AC_NONE,
	6,
	pgsql_help,
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
	"pgsql/help",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
