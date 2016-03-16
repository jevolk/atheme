/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


static
void show_request(sourceinfo_t *const si, const request_t *const request)
{
	static __thread char buf[BUFSIZE];
	request_string(request, buf, sizeof(buf));
	command_success_nodata(si, "%s", buf);
}


int gms_show_group_meta(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	group_t *const group = cmd_parent_data(cmd);

	if(parc < 1)
	{
		void show_line(const char *const key, const char *const val)
		{
			command_success_nodata(si, "%s => %s", key, val);
		}

		if(!group_meta_count(group))
		{
			command_success_nodata(si, "Nothing set.");
			return 1;
		}

		group_meta_foreach(group, show_line);
		return 1;
	}

	const char *const key = parv[0];
	const char *const val = group_meta_get(group, key);
	if(!val)
	{
		gmserr = "Not found.";
		return 0;
	}

	command_success_nodata(si, "%s => %s", key, val);
	return 1;
}


int gms_show_group_contacts(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	char buf[BUFSIZE]; buf[0] = '\0';
	group_t *const group = cmd_parent_data(cmd);
	void append_contact(const void *const user)
	{
		const myuser_t *const mu = user;
		if(!group_access_check(group, mu, GA_CONTACT))
			return;

		mowgli_strlcat(buf, entity(mu)->name, sizeof(buf));
		mowgli_strlcat(buf, ", ", sizeof(buf));
	}

	group_foreach(group, GROUP_USER, append_contact);
	if(strlen(buf))
		command_success_nodata(si, "%s", buf);
	command_success_nodata(si, "\2***\2 End of contacts.", buf);
	return 1;
}


int gms_show_group_members(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	char buf[BUFSIZE]; buf[0] = '\0';
	void append_member(const void *const user)
	{
		const myuser_t *const mu = user;
		mowgli_strlcat(buf, entity(mu)->name, sizeof(buf));
		mowgli_strlcat(buf, ", ", sizeof(buf));
	}

	group_t *const group = cmd_parent_data(cmd);
	group_foreach(group, GROUP_USER, append_member);
	command_success_nodata(si, "%s", buf);
	command_success_nodata(si, "\2***\2 End of members.", buf);
	return 1;
}


int gms_show_group_channels(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	void show_chan(const void *const chan)
	{
		const mychan_t *const mc = chan;
		command_success_nodata(si, "%s", mc->name);
	}

	group_t *const group = cmd_parent_data(cmd);
	group_foreach(group, GROUP_CHAN, show_chan);
	return 1;
}


int gms_show_group_requests(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	size_t cnt = 0;
	void show_line(const void *const request)
	{
		show_request(si, request);
		cnt++;
	}

	group_t *const group = cmd_parent_data(cmd);
	group_foreach(group, GROUP_REQUEST, show_line);

	if(!cnt)
		command_success_nodata(si, "Group has no requests.");

	return 1;
}


int gms_show_group(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		gmserr = "Group is not registered.";
		return 0;
	}

	cmd_set_data(cmd, group);
	return 1;
}


int gms_show_user_meta(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const myuser_t *const user = cmd_parent_data(cmd);

	if(parc < 1)
	{
		void show_line(const char *const key, const char *const val)
		{
			command_success_nodata(si, "%s => %s", key, val);
		}

		if(!user_meta_count(user))
		{
			command_success_nodata(si, "Nothing set.");
			return 1;
		}

		user_meta_foreach(user, show_line);
		return 1;
	}

	const char *const key = parv[0];
	const char *const val = user_meta_get(user, key);
	if(!val)
	{
		gmserr = "Not found.";
		return 0;
	}

	command_success_nodata(si, "%s => %s", key, val);
	return 1;
}


int gms_show_user_requests(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const myuser_t *const user = cmd_parent_data(cmd);

	size_t cnt = 0;
	void show_line(const group_t *const group, const void *const r)
	{
		const request_t *const request = r;
		if(request->user != user)
			return;

		show_request(si, request);
		cnt++;
	}

	groups_elem_foreach(&gms->groups, GROUP_REQUEST, show_line);

	if(!cnt)
		command_success_nodata(si, "You have no requests.");

	return 1;
}


int gms_show_user_groups(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	char buf[BUFSIZE]; buf[0] = '\0';
	void append(const char *const name)
	{
		mowgli_strlcat(buf, name, sizeof(buf));
		mowgli_strlcat(buf, ", ", sizeof(buf));
	}

	const myuser_t *const user = cmd_parent_data(cmd);
	user_meta_list_foreach(user, "groups", append);
	const size_t len = strlen(buf);
	if(len < 3)
	{
		command_success_nodata(si, "Not a member of any groups.");
		return 1;
	}

	// remove the trailing comma+space
	buf[len - 2] = '\0';

	command_success_nodata(si, "Groups: %s", buf);
	return 1;
}


int gms_show_user(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
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


int gms_show_my(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
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


int gms_show_requests(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	void show_request_line(const group_t *const group, const void *const request)
	{
		show_request(si, request);
	}

	groups_elem_foreach(&gms->groups, GROUP_REQUEST, show_request_line);
	command_success_nodata(si, "*** End of requests.");
	return 1;
}


int gms_show_request(cmd_t *cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	const uint id = atoi(parv[0]);
	int req_is_id(const group_t *const group, const void *const request)
	{
		return ((request_t *)request)->id == id;
	}

	const request_t *const request = groups_elem_find(&gms->groups, GROUP_REQUEST, req_is_id);
	if(!request)
	{
		gmserr = "ID was not found.";
		return 0;
	}

	show_request(si, request);
	return 1;
}


cmd_t show = {
"SHOW", NULL, 1, "<command>", NULL,
{
	&(cmd_t){ "GROUP", gms_show_group, 2, "<name> <command>", NULL,
	{
		&(cmd_t){ "META",        gms_show_group_meta                                              },
		&(cmd_t){ "MEMBERS",     gms_show_group_members                                           },
		&(cmd_t){ "CONTACTS",    gms_show_group_contacts                                          },
		&(cmd_t){ "CHANNELS",    gms_show_group_channels                                          },
		&(cmd_t){ "REQUESTS",    gms_show_group_requests                                          },
	}},

	&(cmd_t){ "USER", gms_show_user, 2, "<name> <command>", NULL,
	{
		&(cmd_t){ "META",        gms_show_user_meta, 0, "[key]", user_priv_admin                  },
		&(cmd_t){ "GROUPS",      gms_show_user_groups                                             },
		&(cmd_t){ "REQUESTS",    gms_show_user_requests                                           },
	}},

	&(cmd_t){ "MY", gms_show_my, 1, "<command>", user_priv_auth,
	{
		&(cmd_t){ "META",        gms_show_user_meta, 0, "[key]", user_priv_admin                  },
		&(cmd_t){ "GROUPS",      gms_show_user_groups                                             },
		&(cmd_t){ "REQUESTS",    gms_show_user_requests                                           },
	}},

	&(cmd_t){ "REQUESTS",    gms_show_requests,                                                   },
	&(cmd_t){ "REQUEST",     gms_show_request, 1, "<id>", user_priv_staff                         },
}};


void gms_show(sourceinfo_t *si, int parc, char *parv[])
{
	cmd_call(&show, si, parc, parv);
}


command_t cmd =
{
	"SHOW",
	N_(N_("Show key information.")),
	AC_NONE,
	15,
	gms_show,
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
	GMS_MODULE"/show",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
