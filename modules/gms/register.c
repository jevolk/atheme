/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


typedef struct
{
	request_t self;
	group_t *group;
}
registration_t;


int registration_read(void *const r, database_handle_t *db)
{
	registration_t *const request = r;

	const char *const group_name = db_sread_word(db);
	group_t *const group = gms_find_group(gms, group_name);
	if(!group)
	{
		gmserr = "Group not found.";
		return 0;
	}

	request->group = group;
	return 1;
}


void registration_write(const void *const r, database_handle_t *db)
{
	const registration_t *const request = r;
	const group_t *const group = request->group;
	db_write_word(db, group_name(group));
}


void approve(registration_t *const request)
{
	myuser_t *const user = request(request)->user;
	group_t *const group = request->group;

	group->mode |= GM_APPROVED;

	char buf[BUFSIZE];
	snprintf(buf, sizeof(buf), "Congratulations, your registration of \2%s\2 has been approved!", group_name(group));
	user_send(user, buf);

	group_del(group, GROUP_REQUEST, request);
	request->group = NULL;   // NULL'ing the group pointer is a standard move out of the request's scope, preventing destruction of the group.
	request_delete(request);
}


void reject(registration_t *const request)
{
	myuser_t *const user = request(request)->user;
	group_t *const group = request->group;

	char buf[BUFSIZE];
	snprintf(buf, sizeof(buf), "Sorry, your registration of \2%s\2 has been rejected.", group_name(group));
	user_send(user, buf);

	request_delete(request);
}


void handle_io(void *r, int sig, char *data)
{
	registration_t *const request = r;
	if(irccasecmp(data, "APPROVE") == 0)
		approve(request);
	else if(irccasecmp(data, "REJECT") == 0)
		reject(request);
}



session_code_t submit_registration(interactive_t *const inter, sourceinfo_t *const si)
{
	registration_t *const request = inter->priv;
	group_t *const group = request->group;

	command_success_nodata(si, "Thank you. Your request to register \2%s\2 has been submitted for approval.", group_name(group));

	// NULL'ing the terminate handler removes protection from session/interactive destructor
	inter->terminate = NULL;
	return SESSION_TERMINATE;
}


session_code_t handle_query_visible(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	registration_t *const request = inter->priv;
	group_t *const group = request->group;

	// This is for query_visible
	if(irccasecmp(msg, "Y") == 0)
		group->mode |= GM_LISTED;

	return submit_registration(inter, si);
	//return SESSION_CONTINUE;
}

form_t query_visible =
{
	"Should the group be visible in the primary groups list?",
	handle_query_visible, SESSION_INPUT_BOOL
};


session_code_t handle_url_response(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	session_t *const session = inter->session;
	registration_t *const request = inter->priv;
	group_t *const group = request->group;
	group_meta_set(group, "URL", msg);
	return SESSION_CONTINUE;
}

form_t query_url =
{
	"Please enter the URL:",
	handle_url_response, SESSION_INPUT_ANY, &query_visible
};


session_code_t handle_website_response(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	session_t *const session = inter->session;
	registration_t *const request = inter->priv;
	if(irccasecmp(msg, "Y") != 0)
	{
		command_success_nodata(si, "Your group must have a website...");
		return SESSION_TERMINATE;
	}

	return SESSION_CONTINUE;
}

form_t query_website =
{
	"Does the organization have a website?",
	handle_website_response, SESSION_INPUT_BOOL, &query_url
};


session_code_t handle_email_new_response(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	user_meta_set(si->smu, "email", msg);
	return SESSION_CONTINUE;
}

form_t query_email_new =
{
	"Please enter the e-mail address to use instead:",
	handle_email_new_response, SESSION_INPUT_ANY, &query_website
};


session_code_t handle_ns_email(interactive_t *const inter, sourceinfo_t *const si, char *const msg)
{
	if(irccasecmp(msg, "Y") != 0)
	{
		inter->form = &query_email_new;
		return SESSION_CONTINUE;
	}

	user_meta_set(si->smu, "email", si->smu->email);
	return SESSION_CONTINUE;
}

int test_ns_email(const interactive_t *const inter)
{
	const registration_t *const request = inter->priv;
	return user_meta_get(request(request)->user, "email") != NULL;
}

form_t query_email_ns =
{
	"May I use the e-mail address from your NickServ account?",
	handle_ns_email, SESSION_INPUT_BOOL, &query_website, test_ns_email
};


void abortion_handler(interactive_t *const inter)
{
	registration_t *const request = inter->priv;
	request_delete(request);
}


void dtor(void *const r)
{
	registration_t *const request = r;
	group_t *const group = request->group;

	if(group)
	{
		group_del(group, GROUP_REQUEST, request);
		gms_delete_group(gms, group->name);
	}
}


int ctor(void *const r, void *const d)
{
	registration_t *const request = r;
	const char *const name = d;
	myuser_t *const user = request(request)->user;
	group_t *const group = gms_create_group(gms, name);
	request->group = group;
	if(!group)
	{
		gmserr_prepend("Failed to create group: ");
		return 0;
	}

	group_user_add(group, user);
	group_add(group, GROUP_REQUEST, request);
	access_t *const access = access_new(user, UINT_MAX, "founder");
	if(!group_access_add(group, access))
		access_delete(access);

	const form_t *const form = user->email? &query_email_ns : &query_email_new;
	interactive_t *const inter = interactive_start(&gms->sessions, form, user, request);
	inter->terminate = abortion_handler;

	return 1;
}


const request_vtable_t request_register_table =
{
	"REGISTER",
	sizeof(registration_t),

	.ctor = ctor,
	.dtor = dtor,

	.read = registration_read,
	.write = registration_write,

	.signal[SIGIO] = handle_io
};


void gms_register(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "REGISTER");
		command_fail(si, fault_needmoreparams, _("Syntax: REGISTER <group>"));
		return;
	}

	char *const name = parv[0];
	if(!gms_request(gms, REQUEST_REGISTER, si->smu, name) && gmserr)
		command_fail(si, fault_badparams, "error: %s", gmserr);
}

command_t cmd =
{
	"REGISTER",
	N_(N_("Register a new group.")),
	AC_AUTHENTICATED,
	1,
	gms_register,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	service_bind_command(myservice, &cmd);
	request_vtable[REQUEST_REGISTER] = request_register_table;
}


void module_fini(module_unload_intent_t intent)
{
	service_unbind_command(myservice, &cmd);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	GMS_MODULE"/register",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
