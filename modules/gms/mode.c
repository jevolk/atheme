/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_mode(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "FLAGS");
		command_fail(si, fault_needmoreparams, _("Syntax: MODE <group> [+/-delta]"));
		return;
	}

	const char *const name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, _("Group `%s' is not registered."), name);
		return;
	}

	if(parc == 2)
	{
		const char *const delta = parv[1];
		if(!si->smu)
		{
			command_fail(si, fault_noprivs, _("You are not even identified to nickserv."));
			return;
		}

		if(!group_access_anyof(group, si->smu, GA_FOUNDER|GA_OPERATOR))
		{
			command_fail(si, fault_noprivs, _("You lack sufficient access to change the mode of \2%s\2."), group_name(group));
			return;
		}

		group_mode_delta(group, delta);
	}

	char buf[32];
	mode_mask_to_str(group_mode(group), buf);
	command_success_nodata(si, "+%s \2%s\2 mode mask", buf, group_name(group));
}


command_t cmd =
{
	"MODE",
	N_(N_("Display and change group modes.")),
	AC_NONE,
	2,
	gms_mode,
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
	GMS_MODULE"/mode",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
