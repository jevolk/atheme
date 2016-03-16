/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "atheme.h"

// Service name
#define GMS_NAME                       "GMS"
#define GMS_MODULE                     "gms"
#define GMS_TITLE                      "Group Management System"

#define GMS_DBKEY_CONFIG                GMS_NAME "M"
#define MD_NS_GMS                       "private:gms:"

char *config_defaults[] =
{
    "group_user_max",    "64",
    "group_chan_max",    "32",
    "user_group_max",    "16",
};

service_t *myservice;                  // Convenience initialized by common module init.
__thread const char *gmserr;           // Static error string (i.e errno).
#include "colors.h"
#include "tokens.h"
#include "strvec.h"
#include "mask.h"
#include "util.h"
#include "cmd.h"
#include "meta.h"
meta_t *config;                        // Convenience initialized by common module init.
#include "session.h"
#include "sessions.h"
#include "interactive.h"
#include "access.h"
#include "mode.h"
#include "cloak.h"
#include "user.h"
#include "chan.h"
#include "request.h"
#include "group.h"
#include "groups.h"

typedef struct
{
	meta_t config;                     // System-wide configuration metadata
	size_t reqctr;                     // Request ID counter.
	groups_t groups;                   // Groups collection
	sessions_t sessions;               // State for interactive sessions.

	service_t *svc;
	const struct mode_flag *mode_table;
	const struct access_flag *access_table;
	const group_vtable_t *group_vtable;
	request_vtable_t *request_vtable;
}
GMS;

int gms_request_type_available(const GMS *gms);

int gms_access_delta(GMS *gms, group_t *group, myuser_t *admin, myuser_t *user, const char *flags);
group_t *gms_find_group(GMS *gms, const char *name);
int gms_join_group(GMS *gms, group_t *group, myuser_t *user);
int gms_delete_group(GMS *gms, const char *name);
group_t *gms_create_group(GMS *gms, const char *name);
size_t gms_groups_count(const GMS *gms);

request_t *gms_find_request(GMS *gms, const uint id);
int gms_signal_request(GMS *gms, const uint id, const uint signum, char *data);
int gms_request(GMS *gms, const uint type, myuser_t *user, void *data);

void gms_init(GMS *gms);
void gms_fini(GMS *gms);
void gms_delete(void *gms);
GMS *gms_new();


GMS *gms;  // Primary GMS instance pointer


/* Points various globals in the header files to the extern data in gms.c.
 * Called by module_init_common(); call that instead.
 */
inline
void module_init_extern(GMS *const gms)
{
	myservice = gms->svc;
	config = &gms->config;
	mode_table = gms->mode_table;
	access_table = gms->access_table;
	group_vtable = gms->group_vtable;
	request_vtable = gms->request_vtable;
}


/* Must be called by additional modules.
 */
inline
void module_init_common(module_t *const m)
{
	void *sym_ptr;
	MODULE_TRY_REQUEST_SYMBOL(m, sym_ptr, GMS_MODULE"/gms", GMS_MODULE);
	gms = *((GMS **)sym_ptr);
	module_init_extern(gms);
}


/* Must be called by additional modules when destructing.
 */
inline
void module_fini_common(module_unload_intent_t intent)
{
}


inline
int gms_request(GMS *const gms,
                const uint type,
                myuser_t *const user,
                void *const data)
{
	request_t *const request = request_new(type, user, data);
	if(!request)
		return 0;

	request->id = gms->reqctr++;
	return 1;
}


inline
int gms_signal_request(GMS *const gms,
                       const uint id,
                       const uint signum,
                       char *const data)
{
	request_t *const request = gms_find_request(gms, id);
	if(!request)
	{
		gmserr = "Request ID not found.";
		return 0;
	}

	return request_signal(request, signum, data);
}


inline
request_t *gms_find_request(GMS *const gms,
                            const uint id)
{
	int id_eq(group_t *const group, void *const request)
	{
		return request(request)->id == id;
	}

	return groups_elem_find_mutable(&gms->groups, GROUP_REQUEST, id_eq);
}


inline
size_t gms_groups_count(const GMS *const gms)
{
	return groups_count(&gms->groups);
}


inline
group_t *gms_create_group(GMS *const gms,
                          const char *const name)
{
	if(!group_name_valid(name))
	{
		gmserr = "Name is invalid.";
		return NULL;
	}

	if(groups_find(&gms->groups, name))
	{
		gmserr = "Already exists.";
		return NULL;
	}

	group_t *const group = mowgli_alloc(sizeof(group_t));
	group_init(group);
	group_name_set(group, name);
	group->creation = CURRTIME;
	if(!groups_add(&gms->groups, group))
	{
		gmserr = "Failed to add group to collection.";
		group_fini(group);
		mowgli_free(group);
		return NULL;
	}

	return group;
}


inline
int gms_delete_group(GMS *const gms,
                     const char *const name)
{
	group_t *const group = groups_find_mutable(&gms->groups, name);
	if(!group)
	{
		gmserr = "Not found.";
		return 0;
	}

	group_chan_clear(group);
	group_user_clear(group);
	group_fini(group);
	groups_del(&gms->groups, group);
	mowgli_free(group);
	return 1;
}


inline
int gms_join_group(GMS *const gms,
                   group_t *const group,
                   myuser_t *const user)
{
	int same_user(const void *const elem)
	{
		return user == elem;
	}

	if(group_until(group, GROUP_USER, same_user))
	{
		gmserr = "Already a member of that group.";
		return 0;
	}

	//Note: crossreference between user and group
	group_add(group, GROUP_USER, user);
	group_user_add(group, user);
	return 1;
}


inline
group_t *gms_find_group(GMS *const gms,
                        const char *const name)
{
	return groups_find_mutable(&gms->groups, name);
}


inline
int gms_access_delta(GMS *const gms,
                     group_t *const group,
                     myuser_t *const admin,
                     myuser_t *const user,
                     const char *const flags)
{
	if(!group_access_check(group, admin, GA_FLAGS) && !user->soper)
	{
		gmserr = "Insufficient access to change flags.";
		return 0;
	}

	if(!group_user_has(group, user))
	{
		gmserr = "User is not a member of that group.";
		return 0;
	}

	return group_access_delta(group, user, flags);
}
