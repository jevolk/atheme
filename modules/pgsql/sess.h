/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */

typedef struct sess
{
	user_t *user;
	conn_t *conn;
}
sess_t;

void _sess_result_to_tty(conn_t *const c, const PGresult *const result);
void sess_command(sess_t *const s, sourceinfo_t *si, const char *cmd);
void sess_fini(sess_t *);
int sess_init(sess_t *, user_t *user, conn_t *conn);


inline
int sess_init(sess_t *const s,
              user_t *const user,
              conn_t *const conn)
{
	memset(s, 0x0, sizeof(sess_t));
	s->user = user;
	s->conn = conn;

	if(s->user->myuser != s->conn->owner)
	{
		pgerr = "You do not own this connection.";
		return 0;
	}

	conn->tty = user;
	conn->handle_result = _sess_result_to_tty;
	return 1;
}


inline
void sess_fini(sess_t *const s)
{
	if(!s)
		return;

	if(s->conn)
	{
		conn_tty(s->conn, "*** \2Session Ended\2 ***");

		s->conn->handle_result = NULL;
		s->conn->tty = NULL;
	}
}


inline
void sess_command(sess_t *const s,
                  sourceinfo_t *const si,
                  const char *const cmd)
{
	if(!conn_query(s->conn, cmd))
	{
		command_fail(si, 0, "ERROR: Failed to send query.");
		return;
	}
}


inline
void _sess_result_to_tty(conn_t *const c,
                         const PGresult *const result)
{
	void line_to_tty(const char *const line)
	{
		if(line && line[0])
			conn_tty(c, "%s", line);
	}

	int color = FG_YELLOW;
	const int status = PQresultStatus(result);
	switch(status)
	{
		case PGRES_TUPLES_OK:
		case PGRES_COMMAND_OK:
		case PGRES_SINGLE_TUPLE:
			color = BG_GREEN;
			break;

		case PGRES_FATAL_ERROR:
		case PGRES_BAD_RESPONSE:
			color = BG_RED;
			break;

		case PGRES_COPY_IN:
		case PGRES_COPY_OUT:
		case PGRES_COPY_BOTH:
			color = BG_BLUE;
			break;
	}

	const char *const status_str = reflect_exec(status);
	conn_tty(c, "\2\3%02d,%02d%s\x0f took %ld.%06lds (%d ms) \2\3%02d%s\x0f",
	         FG_WHITE,
	         color,
	         status_str,
	         c->query_tv.tv_sec,
	         c->query_tv.tv_usec,
	         tv2ms(&c->query_tv),
	         FG_RED,
	         PQresultErrorMessage(result));

	debug_result(result, line_to_tty);
}
