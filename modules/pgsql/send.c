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


void pgsql_send(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, "Usage: SEND <id> <query-string>");
		return;
	}

	conn_t *const c = pg_conn_find_mutable(pg, atoi(parv[0]));
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	const int ret = pg_query(pg, c, parv[1]);
	if(!ret)
	{
		command_fail(si, fault_badparams, "#%d: %s", conn_id(c), conn_errmsg(c));
		return;
	}

	command_success_nodata(si, "Query sent to #%d.",
	                       conn_id(c),
	                       conn_errmsg(c));
	conn_flush(c);
}


command_t cmd =
{
	"SEND",
	N_(N_("Send a query to a database.")),
	AC_AUTHENTICATED,
	2,
	pgsql_send,
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
	"pgsql/send",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
