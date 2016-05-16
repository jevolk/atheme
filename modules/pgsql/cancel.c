/*
 * Copyright (c) 2005-2006 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains the OpenSEX (Open Services Exchange) database backend for
 * Atheme. The purpose of OpenSEX is to destroy the old DB format, subjugate its
 * peoples, burn its cities to the ground, and salt the earth so that nothing
 * ever grows there again.
 */

#include "pgsql.h"


void pgsql_cancel(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: CANCEL <id>");
		return;
	}

	conn_t *const c = pg_conn_find_mutable(pg, atoi(parv[0]));
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	char buf[BUFSIZE] = {0};
	if(!conn_query_cancel(c, buf, sizeof(buf)))
	{
		command_success_nodata(si, "CANCEL on #%d: Error sending the cancel: %s",
		                       buf[0]? buf : "and no reason was given.",
		                       conn_id(c));
		return;
	}

	command_success_nodata(si, "CANCEL on #%d: %s",
	                       conn_id(c),
	                       buf[0]? buf : "Cancel was sent but it appears as if nothing was canceled.");
}


command_t cmd =
{
	"CANCEL",
	N_(N_("Send a cancel command to a database.")),
	AC_AUTHENTICATED,
	2,
	pgsql_cancel,
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
	"pgsql/cancel",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
