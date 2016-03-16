/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void gms_info(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "INFO");
		command_fail(si, fault_needmoreparams, _("Syntax: INFO <group>"));
		return;
	}

	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group is not registered.");
		return;
	}

	command_success_nodata(si, "\2***\2 Information about \2%s\2:", group->name);

	#define INFO_LINE "\2%-18s:\2 "

	char mode_buf[32], mode_desc_buf[256];
	mode_mask_to_str(group->mode, mode_buf);
	mask_reflect(&mode_vtable, group->mode, mode_desc_buf, sizeof(mode_desc_buf));
	command_success_nodata(si, INFO_LINE"+%s (%s)",
	                       "MODE",
	                       mode_buf,
	                       mode_desc_buf);

	char creat_buf[64];
	const struct tm *const ct = localtime(&CURRTIME);
	strftime(creat_buf, sizeof(creat_buf), "%c", ct);
	command_success_nodata(si, INFO_LINE"%s (%s ago) (%ld)",
	                       "CREATED",
	                       creat_buf,
	                       time_ago(group->creation),
	                       group->creation);


	if(group_meta_get(group, "URL"))
		command_success_nodata(si, INFO_LINE"%s",
		                       "URL",
		                       group_meta_get(group, "URL"));

	command_success_nodata(si, INFO_LINE"%zu",
	                       "MEMBERS",
	                       group_count(group, GROUP_USER));

	command_success_nodata(si, INFO_LINE"%zu",
	                       "CHANNELS",
	                       group_count(group, GROUP_CHAN));

	command_success_nodata(si, INFO_LINE"%zu",
	                       "REQUESTS",
	                       group_count(group, GROUP_REQUEST));


	command_success_nodata(si, "*** End of Information");
}


command_t cmd =
{
	"INFO",
	N_(N_("Information about a group.")),
	AC_NONE,
	1,
	gms_info,
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
	GMS_MODULE"/info",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
