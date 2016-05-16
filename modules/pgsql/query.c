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


void pgsql_query(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: QUERY <id>");
		return;
	}

	conn_t *const c = pg_conn_find_mutable(pg, atoi(parv[0]));
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	sess_t *const s = pg_sess_new(pg, si->su, c);
	if(!s)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	command_success_nodata(si, "*** \2Session to #%d open\2 *** (Terminate with \2quit\2)", conn_id(c));
}


command_t cmd =
{
	"QUERY",
	N_(N_("Create a query session with a connected database.")),
	AC_AUTHENTICATED,
	1,
	pgsql_query,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	service_bind_command(myservice, &cmd);
}


void module_fini(module_unload_intent_t intent)
{
	// When sessions are created from the QUERY command originating here,
	// they must be destroyed before this module unloads otherwise dtors will dangle.
	pg_sesss_delete(pg);

	service_unbind_command(myservice, &cmd);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	"pgsql/query",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
