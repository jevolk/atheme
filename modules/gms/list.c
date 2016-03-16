/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


void cmd_list(sourceinfo_t *si, int parc, char *parv[])
{
	void list_group(const group_t *const group)
	{
		if((~group_mode(group) & GM_LISTED) && (!si->smu || !si->smu->soper))
			return;

		if((~group_mode(group) & GM_APPROVED) && (!si->smu || !si->smu->soper))
			return;

		const char *desc = group_meta_get(group, "description");
		if(!desc)
			desc = "<no description>";

		char mode_buf[32];
		mode_mask_to_str(group_mode(group), mode_buf);
		command_success_nodata(si, "\2%-20s\2 %8u  +%-10s %-18s : %s",
		                       group_name(group),
		                       group_count(group, GROUP_USER),
		                       mode_buf,
		                       time_ago(group_created(group)),
		                       desc);
	}

	command_success_nodata(si, "%-22s  %-10s %-13s %-20s : %s", "\2NAME\2", "\2MEMBERS\2", "\2MODE\2", "\2AGE\2", "\2DESCRIPTION\2");
	command_success_nodata(si, "%-22s  %-10s %-13s %-20s", "\2--------------------\2", "\2-------\2", "\2----------\2", "\2-----------------\2");

	groups_foreach(&gms->groups, list_group);
	command_success_nodata(si, "*** End of list");
}


command_t cmd =
{
	"LIST",
	N_(N_("Listing of things.")),
	AC_NONE,
	0,
	cmd_list,
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
	GMS_MODULE"/list",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
