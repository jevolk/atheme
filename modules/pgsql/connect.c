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


void pgsql_help_connect(sourceinfo_t *const si, const char *const c)
{
	command_success_nodata(si, "Usage: CONNECT <key=val> [key=val] [...]");
	command_success_nodata(si, "Keys are listed here: http://www.postgresql.org/docs/9.4/static/libpq-connect.html#LIBPQ-PARAMKEYWORDS");
	command_success_nodata(si, "Note: You can use a single URL string as the value of the key dbname=.");
	command_success_nodata(si, "Note: The URL format is documented here: http://www.postgresql.org/docs/9.4/static/libpq-connect.html#LIBPQ-CONNSTRING");
}


void pgsql_connect(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: CONNECT <key=val> [key=val] [...]");
		return;
	}

	const char *key[16] = {0};
	const char *val[16] = {0};
	if(util_parv_to_kv(parc, parv, key, val) < 0)
	{
		command_fail(si, fault_badparams, "Bad parameter string.");
		return;
	}

	conn_t *const c = pg_conn_new(pg, si->smu, key, val);
	if(!c)
	{
		command_fail(si, fault_badparams, "Failed to start connection: %s",
		             pgerr? pgerr : "No error given.");
		return;
	}

	command_success_nodata(si, "#%d: Connecting. Sit tight for a moment and I will call you back...", conn_id(c));
}


void pgsql_squit(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: SQUIT <id>");
		return;
	}

	const uint id = atoi(parv[0]);
	conn_t *const c = pg_conn_find_mutable(pg, id);
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	pg_conn_delete(pg, c);
	command_success_nodata(si, "Closed connection #%u", id);
}


void pgsql_reset(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: RESET <id>");
		return;
	}

	const uint id = atoi(parv[0]);
	conn_t *const c = pg_conn_find_mutable(pg, id);
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	const int ret = pg_conn_reset(pg, c);
	if(!ret)
	{
		command_fail(si, fault_badparams, "Failed to reset #%u: %s", id, pgerr);
		return;
	}

	command_success_nodata(si, "Resetting connection #%u", id);
}


command_t cmd_connect =
{
	"CONNECT",
	N_(N_("Create a connection to a database.")),
	AC_AUTHENTICATED,
	15,
	pgsql_connect,
	{ NULL, pgsql_help_connect }
};


command_t cmd_squit =
{
	"SQUIT",
	N_(N_("Close a connection to a database.")),
	AC_AUTHENTICATED,
	1,
	pgsql_squit,
	{ NULL, NULL }
};


command_t cmd_reset =
{
	"RESET",
	N_(N_("Reset a connection to a database.")),
	AC_AUTHENTICATED,
	1,
	pgsql_reset,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	service_bind_command(myservice, &cmd_connect);
	service_bind_command(myservice, &cmd_squit);
	service_bind_command(myservice, &cmd_reset);
}


void module_fini(module_unload_intent_t intent)
{
	// When connections are created from the CONNECT command originating here,
	// they must be destroyed when this module unloads otherwise dtors will dangle.
	pg_conns_delete(pg);

	service_unbind_command(myservice, &cmd_reset);
	service_unbind_command(myservice, &cmd_squit);
	service_unbind_command(myservice, &cmd_connect);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	"pgsql/connect",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
