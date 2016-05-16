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


void pgsql_list(sourceinfo_t *si, int parc, char *parv[])
{
	size_t cnt = 0;
	void show_conn(const conn_t *const c)
	{
		char cobuf[BUFSIZE]; cobuf[0] = '\0';
		if(c->conn)
		{
			const char *const host = PQhost(c->conn);
			const char *const port = PQport(c->conn);
			snprintf(cobuf, sizeof(cobuf), "\2%-15s\2 @ [%s]:%s",
			         PQdb(c->conn),
			         host?: "unknown",
			         port?: "noport");
		}

		char crbuf[BUFSIZE];
		{
			snprintf(crbuf, sizeof(crbuf), "created %s ago by %s",
			         time_ago(c->creation),
			         c->owner? entity(c->owner)->name : "<nobody>");
		}

		const char *const status = c->conn? reflect_status(PQstatus(c->conn)) : "<DEAD>";
		const enum color_fg status_color = strcmp(status, "OK") == 0? FG_GREEN : FG_RED;
		command_success_nodata(si, "#%-2d \2%c%02d%s%c %35s | %-30s",
		                       conn_id(c),
		                       COLOR_ON,
		                       status_color,
		                       status,
		                       COLOR_OFF,
		                       cobuf,
		                       crbuf);
		cnt++;
	}

	conns_foreach(&pg->conns, show_conn);
	command_success_nodata(si, "*** End of \2%zu\2 Connections ***", cnt);
}


command_t cmd =
{
	"LIST",
	N_(N_("List database connections.")),
	AC_AUTHENTICATED,
	0,
	pgsql_list,
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
	"pgsql/list",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
