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


static
void handle_established(conn_t *const c)
{
	const int status = conn_status(c);
	const char *const msg = reflect_status(status);
	c->connecting = false;

	// When the status handler is set the user is probably not human.
	// Otherwise we send an IRC message indicating the connect result.
	if(!c->handle_status)
		conn_tty(c, "Connection established: \2\3%02d%s\x0f",
		         status == CONNECTION_OK? FG_GREEN : FG_RED,
		         msg);
}


static
void handle_ready(conn_t *const c)
{
	conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
	slog(LG_DEBUG, "PGSQL: #%d: READY: status: %s",
	     conn_id(c),
	     reflect_status(conn_status(c)));

	// Most callbacks are triggered here. c->handle_status is not, so it
	// indicates readiness after any/all results are called back, below.
	conn_process(c);

	// Case to indicate initial connection to user.
	if(c->connecting)
		handle_established(c);

	// When the user is handling this, they control pg_conn_delete().
	// *c may be destroyed by this callback. Must return straight to Atheme.
	if(c->handle_status)
	{
		c->handle_status(c, conn_status(c));
		return;
	}

	if(conn_status(c) != CONNECTION_OK)
	{
		pg_conn_delete(pg, c);
		return;
	}
}


static
void handle_reading(conn_t *const c)
{
	conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
	conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_READ, pg->handler);
}


static
void handle_writing(conn_t *const c)
{
	conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_WRITE, pg->handler);
}


static
void handle_failed(conn_t *const c)
{
	slog(LG_ERROR, "PGSQL: #%d: Connection Failed: %s",
	     conn_id(c),
	     conn_errmsg(c));

	if(c->owner)
		conn_tty(c, "Connection failed: \2\3%02d%s\x0f",
		         FG_RED,
		         conn_errmsg(c));

	if(c->handle_status)
		c->handle_status(c, conn_status(c));
	else
		pg_conn_delete(pg, c);
}


static
void handler(mowgli_eventloop_t *const evl,
             mowgli_eventloop_io_t *const io,
             mowgli_eventloop_io_dir_t dir,
             void *const priv)
{
	conn_t *const c = priv;
	slog(LG_DEBUG, "PGSQL: #%d: [%-6s] status: %s",
	     conn_id(c),
	     reflect_direction(dir),
	     reflect_status(conn_status(c)));

	switch(conn_poll(c))
	{
		case PGRES_POLLING_OK:         handle_ready(c);      break;
		case PGRES_POLLING_READING:    handle_reading(c);    break;
		case PGRES_POLLING_WRITING:    handle_writing(c);    break;
		default:
		case PGRES_POLLING_FAILED:     handle_failed(c);     break;
	}
}


static
void hook_user_delete(user_t *const user)
{
	int same_user(sess_t *const s)
	{
		return s->user == user;
	}

	const size_t ret = pg_sesss_delete_if(pg, same_user);
	if(!ret)
		return;

	slog(LG_INFO, "Terminated %zu query session on user %s delete",
	     ret,
	     user->nick);
}


static
void hook_user_drop(myuser_t *const user)
{
	int is_owner(conn_t *const c)
	{
		return c->owner == user;
	}

	const size_t ret = pg_conns_delete_if(pg, is_owner);
	if(!ret)
		return;

	slog(LG_ERROR, "Removed %zu database connections when dropping account %s",
	     ret,
	     entity(user)->name);
}


static
void handle_command(sourceinfo_t *const si, int parc, char **parv)
{
	if(parc < 2)
		return;

	char *const text = parv[1];
	if(!text || !text[0])
		return;

	pgerr = ""; // Reset the errno'ish on each command entry

	sess_t *const sess = pg_sess_find_mutable(pg, si->su);
	if(sess && irccasecmp(text, "QUIT") == 0)
	{
		pg_sess_delete(pg, sess);
		return;
	}
	else if(sess)
	{
		sess_command(sess, si, text);
		return;
	}

	char *ctx, *const cmd = strtok_r(text, " ", &ctx);
	if(!cmd || !cmd[0])
		return;

	char *const remain = strtok_r(NULL, "", &ctx);
	if(cmd[0] == '\001')
	{
		handle_ctcp_common(si, cmd, remain);
		return;
	}

	command_exec_split(myservice, si, cmd, remain, myservice->commands);
}


static
void pg_init(PG *const pg)
{
	pgerr = "";
	pg->svc = myservice = service_add("pgsql", handle_command);
	pg->handler = &handler;
	hook_add_user_drop(hook_user_drop);
	hook_add_user_delete(hook_user_delete);
}


static
void pg_fini(PG *const pg)
{
	pg_sesss_delete(pg);
	pg_conns_delete(pg);
	hook_del_user_delete(hook_user_delete);
	hook_del_user_drop(hook_user_drop);
	service_delete(pg->svc);
}


void module_init(module_t *const m)
{
	pg = mowgli_alloc(sizeof(PG));
	pg_init(pg);
}


void module_fini(module_unload_intent_t intent)
{
	pg_fini(pg);
	mowgli_free(pg);
}


DECLARE_MODULE_V1
(
	"pgsql/pgsql",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
