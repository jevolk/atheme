/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_flags(sourceinfo_t *si, int parc, char *parv[])
{
	void show_access(const void *const ent)
	{
		char flags[64], flags_desc[256];
		const access_t *const acc = ent;
		mask_to_str(&access_vtable, acc->mask, flags);
		mask_reflect(&access_vtable, acc->mask, flags_desc, sizeof(flags_desc));
		command_success_nodata(si, "%-18s  %16s ago.  %-18s  +%-18s (%s)",
		                       entity(acc->user)->name,
		                       time_ago(acc->modified),
		                       strlen(acc->role)? acc->role : "none",
		                       flags,
		                       flags_desc);
	}

	void show_header(void)
	{
		command_success_nodata(si, "%-20s  %-23s  %-20s  %-21s", "\2ACCOUNT\2", "\2MODIFIED\2", "\2ROLE\2", "\2FLAGS\2");
		command_success_nodata(si, "%-20s  %-23s  %-20s  %-21s", "\2------------------\2", "\2---------------------\2", "\2------------------\2", "\2------------------\2");
	}

	void show_footer(void)
	{
		command_success_nodata(si, "*** End of list");
	}

	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "FLAGS");
		command_fail(si, fault_needmoreparams, _("Syntax: FLAGS <group> [user <+delta>]"));
		return;
	}

	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, _("Group `%s' is not registered."), group_name);
		return;
	}

	if(parc < 2)
	{
		show_header();
		group_foreach(group, GROUP_ACCESS, show_access);
		show_footer();
		return;
	}

	const char *const user_name = parv[1];
	myuser_t *const user = myuser_find(user_name);
	if(!user)
	{
		command_fail(si, fault_nosuch_target, "User `%s' was not found.", user_name);
		return;
	}

	if(parc < 3)
	{
		const access_t *const acc = group_access_find(group, user);
		if(!acc)
		{
			command_fail(si, fault_nosuch_target, _("User `%s' has no access to \2%s\2."), entity(user)->name, group->name);
			return;
		}

		show_access(acc);
		return;
	}

	const char *const flags = parv[2];
	if(!gms_access_delta(gms, group, si->smu, user, flags))
	{
		command_fail(si, fault_badparams, _("error: %s"), gmserr);
		return;
	}

	if(parc >= 4)
	{
		const char *const role = parv[3];
		access_t *const acc = group_access_find_mutable(group, user);
		if(acc)
			mowgli_strlcpy(acc->role, role, sizeof(acc->role));
	}

	const access_t *const acc = group_access_find(group, user);
	show_access(acc);
}


command_t cmd =
{
	"FLAGS",
	N_(N_("Change access permissions.")),
	AC_AUTHENTICATED,
	4,
	gms_flags,
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
	GMS_MODULE"/flags",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
