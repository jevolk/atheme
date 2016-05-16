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


void pgsql_info(sourceinfo_t *si, int parc, char *parv[])
{
	if(!parc)
	{
		command_fail(si, fault_needmoreparams, "Usage: INFO <id>");
		command_fail(si, fault_needmoreparams, "I have %zu connections available.", conns_count(&pg->conns));
		return;
	}

	const conn_t *const c = pg_conn_find(pg, atoi(parv[0]));
	if(!c)
	{
		command_fail(si, fault_badparams, "%s", pgerr);
		return;
	}

	#define INFO_LINE "\2%-18s:\2 "
	command_success_nodata(si, "Information for connection #\2%d\2", conn_id(c));
	{
		const char *const status = c->conn? reflect_status(conn_status(c)) : "<DEAD>";
		const enum color_fg status_color = strcmp(status, "OK") == 0?      FG_GREEN:
		                                   strcmp(status, "<DEAD>") == 0?  FG_RED:
		                                                                   FG_YELLOW;
		command_success_nodata(si, INFO_LINE"\2%c%02d%s%c",
		                       "STATUS",
		                       COLOR_ON,
		                       status_color,
		                       status,
		                       COLOR_OFF);
	}
	if(c->conn)
	{
		const char *const status = reflect_trans(conn_trans_status(c));
		const enum color_fg status_color = strcmp(status, "INERROR") == 0? FG_RED : FG_GREEN;
		command_success_nodata(si, INFO_LINE"\2%c%02d%s%c",
		                       "ACTION",
		                       COLOR_ON,
		                       status_color,
		                       status,
		                       COLOR_OFF);
	}
	{
		command_success_nodata(si, INFO_LINE"SENT[%lu] RESULTS[%lu]",
			                   "QUERIES",
			                   c->queries_sent,
			                   c->results_recv);
	}
	{
		char create_buf[64];
		const struct tm *const ct = localtime(&c->creation);
		strftime(create_buf, sizeof(create_buf), "%c", ct);
		command_success_nodata(si, INFO_LINE"%s (%ld)",
		                       "CREATED",
		                       create_buf,
		                       c->creation);
	}
	{
		command_success_nodata(si, INFO_LINE"%s",
		                       "OWNER",
		                       c->owner? entity(c->owner)->name : "<UNAVAILABLE>");
	}
	if(c->tty)
	{
		command_success_nodata(si, INFO_LINE"%s",
		                       "SESSION",
		                       c->tty->nick);
	}
	if(c->conn)
	{
		const char *const hn = PQhost(c->conn);
		if(hn)
			command_success_nodata(si, INFO_LINE"%s",
			                       "DBHOST",
			                       hn);
	}
	if(c->conn)
	{
		command_success_nodata(si, INFO_LINE"%s",
		                       "DBPORT",
		                       PQport(c->conn)?: "<UNAVAILABLE>");
	}
	if(c->conn)
	{
		command_success_nodata(si, INFO_LINE"%s",
		                       "DBNAME",
		                       PQdb(c->conn)?: "<UNAVAILABLE>");
	}
	if(c->conn)
	{
		command_success_nodata(si, INFO_LINE"%s",
		                       "DBUSER",
		                       PQuser(c->conn)?: "<UNAVAILABLE>");
	}
	if(c->conn)
	{
		const char *const dbopts = PQoptions(c->conn);
		if(dbopts && dbopts[0])
			command_success_nodata(si, INFO_LINE"%s",
			                       "DBOPTS",
			                       dbopts);
	}
	if(c->conn)
	{
		command_success_nodata(si, INFO_LINE"protocol[%d] server[%d] library[%d]",
			                   "VERSION",
			                    PQprotocolVersion(c->conn),
			                    PQserverVersion(c->conn),
			                    PQlibVersion());
	}

	command_success_nodata(si, "*** End of Information ***");
}


command_t cmd =
{
	"INFO",
	N_(N_("Display information about a database connection.")),
	AC_NONE,
	1,
	pgsql_info,
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
	"pgsql/info",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
