/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"

session_code_t handle_confirmation(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	if(irccasecmp(msg, "Y") != 0)
		return SESSION_TERMINATE;

	char *const group_name = inter->priv;
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group is not registered.");
		return SESSION_TERMINATE;
	}

	if(!group_user_del(group, si->smu))
	{
		command_fail(si, fault_badparams, "Failed to remove you from \2%s\2: %s", group->name, gmserr);
		return SESSION_TERMINATE;
	}

	command_success_nodata(si, "You have left \2%s\2.", group->name);
    return SESSION_TERMINATE;
}


void confirmation_dtor(interactive_t *const inter)
{
	char *const group_name = inter->priv;
	mowgli_free(group_name);
}


form_t confirmation =
{
	"Are you sure you want to leave the group?", handle_confirmation, SESSION_INPUT_BOOL
};


void gms_part(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "PART");
		command_fail(si, fault_needmoreparams, _("Syntax: part <group>"));
		return;
	}

	const char *const name = parv[0];
	const group_t *const group = gms_find_group(gms, name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group `%s' is not registered.", name);
		return;
	}

	if(!group_user_has(group, si->smu))
	{
		command_fail(si, fault_nosuch_target, "You are not a member of \2%s\2.", group_name(group));
		return;
	}

	interactive_t *const inter = interactive_start(&gms->sessions, &confirmation, si->smu, mowgli_strdup(name));
	inter->terminate = confirmation_dtor;
}


command_t cmd =
{
	"PART",
	N_(N_("Part ways with a group.")),
	AC_AUTHENTICATED,
	1,
	gms_part,
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
	GMS_MODULE"/part",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
