/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


int gms_set_group_url(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	group_t *const group = cmd_parent_data(cmd);
	const char *const value = parv[0];
	group_meta_set(group, "URL", value);
	command_success_nodata(si, "success.");
	return 1;
}


int gms_set_group_meta(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	group_t *const group = cmd_parent_data(cmd);
	const char *const key = parv[0];
	const char *const value = parv[1];
	if(!value || !strlen(value))
	{
		group_meta_del(group, key);
		command_success_nodata(si, "key deleted.");
		return 1;
	}

	group_meta_set(group, key, value);
	command_success_nodata(si, "success.");
	return 1;
}


int gms_set_group(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		static __thread char errbuf[BUFSIZE];
		snprintf(errbuf, sizeof(errbuf), "Group `%s' is not registered.", group_name);
		gmserr = errbuf;
		return 0;
	}

	cmd_set_data(cmd, group);
	return 1;
}


int gms_set_user_phone(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const value = parv[0];
	myuser_t *const user = cmd_parent_data(cmd);
	user_meta_set(user, "PHONE", value);
	command_success_nodata(si, "success.");
	return 1;
}


int gms_set_user_email(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const value = parv[0];
	myuser_t *const user = cmd_parent_data(cmd);
	user_meta_set(user, "EMAIL", value);
	command_success_nodata(si, "success.");
	return 1;
}


int gms_set_user_meta(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const key = parv[0];
	const char *const value = parv[1];
	myuser_t *const user = cmd_parent_data(cmd);
	if(!value || !strlen(value))
	{
		user_meta_del(user, key);
		command_success_nodata(si, "key deleted.");
		return 1;
	}

	user_meta_set(user, key, value);
	command_success_nodata(si, "success.");
	return 1;
}


int gms_set_user(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const user_name = parv[0];
	myuser_t *const user = myuser_find(user_name);
	if(!user)
	{
		gmserr = "User is not registered.";
		return 0;
	}

	cmd_set_data(cmd, user);
	return 1;
}


int gms_set_my(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	myuser_t *const user = si->smu;
	if(!user)
	{
		gmserr = "You are not logged in.";
		return 0;
	}

	cmd_set_data(cmd, user);
	return 1;
}


cmd_t set = {
"SET", NULL, 1, "<command>", user_priv_auth,
{
	&(cmd_t){ "MY", gms_set_my, 2, "<command> <value>", NULL,
	{
		&(cmd_t){ "EMAIL",  gms_set_user_email,    1, "<address>"                        },
		&(cmd_t){ "PHONE",  gms_set_user_phone,    1, "<number>"                         },
	}},

	&(cmd_t){ "USER", gms_set_user, 2, "<command> <value>", user_priv_admin,
	{
		&(cmd_t){ "EMAIL",  gms_set_user_email,    1, "<address>"                        },
		&(cmd_t){ "PHONE",  gms_set_user_phone,    1, "<number>"                         },
		&(cmd_t){ "META",   gms_set_user_meta,     1, "<key> [val]",                     },
	}},

	&(cmd_t){ "GROUP", gms_set_group, 2, "<group name> <command>", NULL,
	{
		&(cmd_t){ "URL",    gms_set_group_url,     1, "<url>"                            },
		&(cmd_t){ "META",   gms_set_group_meta,    2, "<key> [value]", user_priv_admin   },
	}},
}};


void gms_set(sourceinfo_t *si, int parc, char *parv[])
{
	cmd_call(&set, si, parc, parv);
}


void gms_help_set(sourceinfo_t *si, const char *sc)
{

}


command_t cmd =
{
	"SET",
	N_(N_("Update key information.")),
	AC_AUTHENTICATED,
	15,
	gms_set,
	{ NULL, gms_help_set }
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
	GMS_MODULE"/set",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
