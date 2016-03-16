	/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"

//TODO: config
const size_t cloak_limit = 3;


session_code_t handle_confirmation(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	if(irccasecmp(msg, "Y") != 0)
	{
		user_meta_del(si->smu, "CLOAK-UNCF");
		return SESSION_TERMINATE;
	}

	const char *const cloak = user_meta_get(si->smu, "CLOAK-UNCF");
	command_success_nodata(si, "Setting your cloak to [%s]", cloak);
	user_cloak_set(si->smu, cloak);
	user_meta_del(si->smu, "CLOAK-UNCF");
	return SESSION_TERMINATE;
}

void confirmation_generator(interactive_t *const inter, char *const buf, size_t max)
{
	const char *const cloak = user_meta_get(inter->session->user, "CLOAK-UNCF");
	snprintf(buf, max, "Are you sure you want to change your cloak to \2%s\2?", cloak?: "????");
}

form_t confirmation =
{
	NULL, handle_confirmation, SESSION_INPUT_BOOL, NULL, NULL, confirmation_generator
};


void gms_cloak(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_success_nodata(si, "This command allows you to customize your hostmask.");
		command_success_nodata(si, "The following cloaks are available:");
		command_success_nodata(si, "");

		uint i = 1;
		void each(const group_t *const group)
		{
			const char *const cloak = group_access_cloak(group, si->smu);
			if(!cloak || !strlen(cloak))
				return;

			command_success_nodata(si, "%u.  %s.%s/", i, group->name, cloak);
			i++;
		}

		groups_user_foreach(&gms->groups, si->smu, each);

		if(i == 1)
		{
			command_success_nodata(si, "You have no cloaks authorized, sorry.");
			return;
		}

		command_success_nodata(si, "");
		command_success_nodata(si, "Enter up to %u numbers in the order you wish them to appear.", cloak_limit);
		command_success_nodata(si, "Enter 0 to clear everything.");
		command_success_nodata(si, "");
		command_success_nodata(si, "Example: /msg gms cloak 2 1 3");
		return;
	}

	uint i = 0;
	uint selection[16] = {0};
	for(; i < parc && i < 16 && i < cloak_limit; i++)
		selection[i] = atoi(parv[i]);

	if(i == 1 && selection[0] == 0)
	{
		command_success_nodata(si, "Removing your cloak...");
		user_cloak_unset(si->smu);
		return;
	}

	uint j = 0, k = 0;
	char buf[BUFSIZE] = {0};
	void each_group(const group_t *const group)
	{
		const char *const cloak = group_access_cloak(group, si->smu);
		if(!cloak || !strlen(cloak))
			return;

		if(++j == selection[k])
		{
			cloak_append(buf, sizeof(buf), group->name, cloak);
			selection[k] = 0;
			++k;
		}
	}

	while(k < i)
	{
		j = 0;
		uint l = k;
		groups_user_foreach(&gms->groups, si->smu, each_group);
		if(!j || l == k)
		{
			command_success_nodata(si, "You have nothing to set.");
			return;
		}
	}

	mowgli_strlcat(buf, entity(si->smu)->name, sizeof(buf));
	user_meta_set(si->smu, "CLOAK-UNCF", buf);
	interactive_start(&gms->sessions, &confirmation, si->smu, NULL);
}


command_t cmd =
{
	"CLOAK",
	N_(N_("Manage and display hostname cloaks.")),
	AC_AUTHENTICATED,
	15,
	gms_cloak,
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
	GMS_MODULE"/cloak",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
