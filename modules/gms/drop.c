/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_drop(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "DROP");
		command_fail(si, fault_needmoreparams, _("Syntax: drop <group>"));
		return;
	}

	const char *const name = parv[0];
	const group_t *const group = groups_find(&gms->groups, name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group `%s' is not registered.", name);
		return;
	}

	if(!group_access_check(group, si->smu, GA_FOUNDER))
	{
		command_success_nodata(si, "You lack access to drop \2%s\2.", group->name);
		return;
	}

	if(!gms_delete_group(gms, name))
	{
		command_success_nodata(si, "error: %s", gmserr);
		return;
	}

	command_success_nodata(si, "Group `\2%s\2' has been dropped.", name);
}


command_t cmd =
{
	"DROP",
	N_(N_("Delete a group.")),
	AC_AUTHENTICATED,
	1,
	gms_drop,
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
	GMS_MODULE"/drop",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
