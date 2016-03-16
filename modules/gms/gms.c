/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


mowgli_list_t gms_conf_table;
request_vtable_t _request_vtable[GMS_REQUEST_TYPES_MAX];      // request.h
group_vtable_t _group_vtable[_GROUP_ATTRS_];                  // group.h

struct mode_flag _mode_table[256] =                           // mode.h
{
	['A'] = { GM_APPROVED,        "APPROVED"       },
	['L'] = { GM_LISTED,          "LISTED"         },
	['p'] = { GM_PRIVATE,         "PRIVATE"        },
};

struct access_flag _access_table[256] =                       // access.h
{
    ['F'] = { GA_FOUNDER,         "FOUNDER"        },
    ['S'] = { GA_SUCCESSOR,       "SUCCESSOR"      },
    ['C'] = { GA_CONTACT,         "CONTACT"        },
    ['f'] = { GA_FLAGS,           "FLAGS"          },
    ['O'] = { GA_CHANADMIN,       "CHANADMIN"      },
    ['U'] = { GA_USERADMIN,       "USERADMIN"      },
    ['o'] = { GA_OPERATOR,        "OPERATOR"       },
    ['i'] = { GA_INVITE,          "INVITE"         },
    ['s'] = { GA_SET,             "SET"            },
    ['c'] = { GA_CLOAK,           "CLOAK"          },
};


static
void set_config_defaults(meta_t *const meta)
{
	void set_meta(const char *const key, const char *const val)
	{
		meta_set(meta, key, val);
	}

	config_defaults_foreach(set_meta);
}


static
void write_config(database_handle_t *const db)
{
	meta_write(&gms->config, db);
}


static
void read_config(database_handle_t *const db,
                 const char *const type)
{
	meta_read(&gms->config, db, type);
}


static
void write_groups(database_handle_t *const db)
{
	groups_write(&gms->groups, db);
}


static
void read_group(database_handle_t *const db,
                const char *const type)
{
	groups_read(&gms->groups, db, type);
}


static
void read_user(myuser_t *const user)
{
	void each(const char *const group_name)
	{
		group_t *const group = groups_find_mutable(&gms->groups, group_name);
		if(!group)
			return;

		group_add(group, GROUP_USER, user);
	}

	user_meta_list_foreach(user, "groups", each);
}


static
void hook_db_read(void *d)
{
	{
		void *md;
		myentity_iteration_state_t state;
		MYENTITY_FOREACH(md, &state)
			if(user(md))
				read_user(md);
	}

	{
		mychan_t *mc;
		group_t *group;
		mowgli_patricia_iteration_state_t state;
		MOWGLI_PATRICIA_FOREACH(mc, &state, mclist)
			if((group = groups_find_by_channame_mutable(&gms->groups, mc->name)))
				group_add(group, GROUP_CHAN, mc);
	}
}


static
void hook_user_drop(myuser_t *const user)
{
	void each(char *const group_name)
	{
		group_t *const group = groups_find_mutable(&gms->groups, group_name);
		if(!group)
			return;

		group_user_del(group, user);
	}

	// End any sessions for this account.
	sessions_end_session(&gms->sessions, user);

	// Normal removal of a user from a group modifies the user's metadata.
	// To safetly remove all of the groups in an iteration, we must copy the
	// metadata string first and iterate over that.
	const char *const group_list_cur = user_meta_get(user, "groups");
	if(!group_list_cur)
		return;

	char *group_list scope(mowgli_free) = mowgli_strdup(group_list);
	tokens_mutable(group_list, GMS_MDU_LIST_SEP_STR, each);
}


static
void hook_channel_drop(mychan_t *const chan)
{
	group_t *const group = groups_find_by_channame_mutable(&gms->groups, chan->name);
	if(!group)
		return;

	group_del(group, GROUP_CHAN, chan);
}


static
void hook_channel_register(hook_channel_req_t *const data)
{
	mychan_t *const chan = data->mc;
	group_t *const group = groups_find_by_channame_mutable(&gms->groups, chan->name);
	if(!group)
		return;

	group_chan_acquire(group, chan);
}


static
void hook_channel_can_register(hook_channel_register_check_t *const data)
{
	sourceinfo_t *const si = data->si;
	const char *const name = data->name;
	const group_t *const group = groups_find_by_channame(&gms->groups, name);
	if(!group)
	{
		command_fail(si, fault_noprivs,
		             _("\2%s\2 must be registered with GMS. Try /msg GMS help. For more information, see http://freenode.net/policy.shtml#channelnaming"),
	                 name);

		data->approved = -1;
		return;
	}

	if(!group_access_check(group, si->smu, GA_CHANADMIN) && !si->smu->soper)
	{
		command_fail(si, fault_noprivs,
		             _("\2%s\2 is reserved. Only authorized members of \2%s\2 can register it."),
	                 name,
	                 group->name);

		data->approved = -1;
		return;
	}
}


void gms_handler(sourceinfo_t *const si, int parc, char **parv)
{
	if(parc < 2)
		return;

	char *const text = parv[1];

	// If this message is handled by the session system there is nothing else
	// to do here. Otherwise proceed normally.
	if(sessions_handle(&gms->sessions, si, text))
		return;

	char *ctx;
	char *const cmd = strtok_r(text, " ", &ctx);
	if(!cmd || !strlen(cmd))
		return;

	char *const remain = strtok_r(NULL, "", &ctx);
	if(cmd[0] == '\001')
	{
		handle_ctcp_common(si, cmd, remain);
		return;
	}

	command_exec_split(myservice, si, cmd, remain, myservice->commands);
}


void gms_init(GMS *const gms)
{
	memset(gms, 0x0, sizeof(GMS));

	gms->svc = service_add(GMS_NAME, gms_handler);

	// Provide all the extern symbols residing here
	gms->mode_table = _mode_table;
	gms->access_table = _access_table;
	gms->request_vtable = _request_vtable;
	gms->group_vtable = _group_vtable;
	group_vtable_init(_group_vtable);
	module_init_extern(gms);

	meta_init(&gms->config, GMS_DBKEY_CONFIG);
	db_register_type_handler(GMS_DBKEY_CONFIG, &read_config);
	hook_add_db_write(write_config);
	set_config_defaults(&gms->config);

	groups_init(&gms->groups);

	hook_add_db_write(write_groups);
	db_register_type_handler(GMS_DBKEY_GROUP, &read_group);
	for(size_t i = 0; i < _GROUP_ATTRS_; i++)
	{
		static char buf[_GROUP_ATTRS_][16];
		gms_dbkey_group(i, buf[i]);
		db_register_type_handler(buf[i], &read_group);
	}

	hook_add_db_read(hook_db_read);
	hook_add_user_drop(hook_user_drop);
	hook_add_channel_drop(hook_channel_drop);
	hook_add_channel_register(hook_channel_register);
	hook_add_channel_can_register(hook_channel_can_register);

	sessions_init(&gms->sessions);
}


void gms_fini(GMS *const gms)
{
	sessions_fini(&gms->sessions);

	hook_del_channel_can_register(hook_channel_can_register);
	hook_del_channel_register(hook_channel_register);
	hook_del_channel_drop(hook_channel_drop);
	hook_del_user_drop(hook_user_drop);
	hook_del_db_read(hook_db_read);

	for(size_t i = 0; i < _GROUP_ATTRS_; i++)
	{
		char buf[16];
		gms_dbkey_group(i, buf);
		db_unregister_type_handler(buf);
	}
	db_unregister_type_handler(GMS_DBKEY_GROUP);
	hook_del_db_write(write_groups);

	groups_fini(&gms->groups);

	hook_del_db_write(write_config);
	db_unregister_type_handler(GMS_DBKEY_CONFIG);
	meta_fini(&gms->config);

	service_delete(gms->svc);
}


GMS *gms_new(void)
{
	GMS *const gms = mowgli_alloc(sizeof(GMS));
	gms_init(gms);
	return gms;
}


void gms_delete(void *const g)
{
	GMS *const gms = g;
	gms_fini(gms);
	mowgli_free(gms);
}


void module_init(module_t *const m)
{
	gms = gms_new();
	//hook_db_read(NULL);  // for module RELOAD when it works.
}


void module_fini(module_unload_intent_t intent)
{
	gms_delete(gms);
	gms = NULL;
}


DECLARE_MODULE_V1
(
	GMS_MODULE"/gms",
	// MODULE_UNLOAD_CAPABILITY_NEVER,  /* Can't reload without DB keys reread */
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
