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


void pgsql_ping(sourceinfo_t *si, int parc, char *parv[])
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, "Usage: PING <key=val> [key=val] [...]");
		command_fail(si, fault_needmoreparams, "Note: PING parameters are analogous to CONNECT.");
		command_fail(si, fault_needmoreparams, "Note: PING is synchronous and may/will block the server. Use it with care.");
		return;
	}

	const char *key[16];
	const char *val[16];
	if(util_parv_to_kv(parc, parv, key, val) < 0)
	{
		command_fail(si, fault_badparams, "Bad parameter string.");
		return;
	}

	const char *res;
	const int ret = pg_ping(pg, key, val, &res);
	command_success_nodata(si, "PONG: %s", res);
}


command_t cmd =
{
	"PING",
	N_(N_("Ping a database.")),
	AC_SRA,
	15,
	pgsql_ping,
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
	"pgsql/ping",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
