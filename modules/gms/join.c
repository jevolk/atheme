/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"

typedef struct
{
    request_t super;

}
request_join_t;

/*
int request_join_ctor(request_t *const request, void *const data)
{
	return 1;
}


void request_join_dtor(void *const r)
{

}


void request_join_signal(void *const req, int sig)
{
	request_t *const r = req;
	notice(myservice->me->nick, entity(r->user)->name, "Signal %d", sig);
}
*/

request_vtable_t request_join_table =
{
	"JOIN",
	sizeof(request_join_t),
//	.ctor = request_join_ctor,
//	.dtor = request_join_dtor,
//	.signal[SIGALRM] = request_join_signal,
};



void gms_join(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "JOIN");
		command_fail(si, fault_needmoreparams, _("Syntax: JOIN <group>"));
		return;
	}

	const char *const group_name = parv[0];
	group_t *const group = groups_find_mutable(&gms->groups, group_name);
	if(!group)
	{
		command_fail(si, fault_nosuch_target, "Group is not registered.");
		return;
	}

	if(!gms_request(gms, REQUEST_JOIN, si->smu, NULL) && gmserr)
		command_fail(si, fault_badparams, "Failed: %s", gmserr);
}


command_t cmd =
{
	"JOIN",
	N_(N_("Join a group.")),
	AC_AUTHENTICATED,
	1,
	gms_join,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	service_bind_command(myservice, &cmd);
	request_vtable[1] = request_join_table;
}


void module_fini(module_unload_intent_t intent)
{
	service_unbind_command(myservice, &cmd);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	GMS_MODULE"/join",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
