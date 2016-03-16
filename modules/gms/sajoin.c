/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_sajoin(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SAJOIN");
		command_fail(si, fault_needmoreparams, _("Syntax: SAJOIN <group> [user]"));
		return;
	}

	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group is not registered.");
		return;
	}

	myuser_t *const user = parc < 2? si->smu : myuser_find(parv[1]);
	if(!user)
	{
		command_fail(si, fault_nosuch_target, "User not found.");
		return;
	}

	if(!gms_join_group(gms, group, user))
	{
		command_fail(si, fault_badparams, "%s", gmserr);
		return;
	}

	command_success_nodata(si, "Success joined user \2%s\2 into group \2%s\2", entity(user)->name, group->name);
}


command_t cmd =
{
	"SAJOIN",
	N_(N_("Services admin manual join.")),
	AC_SRA,
	2,
	gms_sajoin,
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
	GMS_MODULE"/sajoin",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
