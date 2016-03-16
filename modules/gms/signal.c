/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_signal(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SIGNAL");
		command_fail(si, fault_needmoreparams, _("Syntax: SIGNAL <signum> <id> [data]"));
		return;
	}

	const uint signum = abs(atoi(parv[0]));
	if(signum >= 32)
	{
		command_fail(si, fault_nosuch_target, "Invalid signal number.");
		return;
	}

	const uint id = atoi(parv[1]);
	char *const data = parc > 2? parv[2] : NULL;
	if(gms_signal_request(gms, id, signum, data))
		command_success_nodata(si, "Sent signal %u to %u.", signum, id);
	else if(gmserr)
		command_fail(si, fault_nosuch_target, "error: %s", gmserr);
	else
		command_fail(si, fault_nosuch_target, "Signal was not handled by %u.", id);
}


command_t cmd =
{
	"SIGNAL",
	N_(N_("Send a signal to something.")),
	AC_SRA,
	3,
	gms_signal,
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
	GMS_MODULE"/signal",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
