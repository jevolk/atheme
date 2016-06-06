/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */

struct conn;
typedef void (*conn_handle_result_t)(struct conn *, const PGresult *);
typedef void (*conn_handle_notice_t)(struct conn *, const PGresult *, const char *msg);
typedef void (*conn_handle_notify_t)(struct conn *, const char *name, const int pid, const char *val);
typedef int (*conn_handle_event_any_t)(struct conn *, const PGEventId, void *data);
typedef int (*conn_handle_event_register_t)(struct conn *, PGEventRegister *);
typedef int (*conn_handle_event_conn_reset_t)(struct conn *, PGEventConnReset *);
typedef int (*conn_handle_event_conn_destroy_t)(struct conn *, PGEventConnDestroy *);
typedef int (*conn_handle_event_result_copy_t)(struct conn *, PGEventResultCopy *);
typedef int (*conn_handle_event_result_create_t)(struct conn *, PGEventResultCreate *);
typedef int (*conn_handle_event_result_destroy_t)(struct conn *, PGEventResultDestroy *);
typedef struct conn
{
	int id;
	time_t creation;
	myuser_t *owner;
	user_t *tty;
	PGconn *conn;
	void *priv;                                  // available to software
	mowgli_eventloop_pollable_t *pollable;
	bool connecting;
	uint64_t queries_sent;
	uint64_t results_recv;
	uint64_t notifies_recv;
	struct timeval query_tv;

	conn_handle_event_result_destroy_t handle_event_result_destroy;
	conn_handle_event_result_create_t handle_event_result_create;
	conn_handle_event_result_copy_t handle_event_result_copy;
	conn_handle_event_conn_destroy_t handle_event_conn_destroy;
	conn_handle_event_conn_reset_t handle_event_conn_reset;
	conn_handle_event_register_t handle_event_register;
	conn_handle_event_any_t handle_event_any;
	conn_handle_notify_t handle_notify;
	conn_handle_notice_t handle_notice;
	conn_handle_result_t handle_result;
}
conn_t;

// Observers
int conn_id(const conn_t *);
int conn_busy(const conn_t *);
ConnStatusType conn_status(const conn_t *);
PGTransactionStatusType conn_trans_status(const conn_t *);
const char *conn_errmsg(const conn_t *);

// Utils
int conn_tty(conn_t *, const char *fmt, ...) PRINTFLIKE(2, 3);

// Processing stack
int conn_consume(conn_t *);
PGresult *conn_get_result(conn_t *);
PGnotify *conn_get_notify(conn_t *);
ssize_t conn_process_notify(conn_t *);
ssize_t conn_process_result(conn_t *);
size_t conn_process(conn_t *);

// Internal callback dispatch
void _conn_result_notice_handler(void *conn, const PGresult *);
int _conn_event_handler(PGEventId id, void *info, void *conn);

// Attribute mutators
int conn_set_iocb(conn_t *, const mowgli_eventloop_io_dir_t, mowgli_eventloop_io_cb_t *);

// Primary control
int conn_query_cancel(conn_t *, char *errbuf, const size_t bufmax);
int conn_query(conn_t *, const char *query);
int conn_reset_poll(conn_t *);
int conn_poll(conn_t *);
int conn_reset(conn_t *);
int conn_flush(conn_t *);

// ctor / dtor
void conn_fini(conn_t *);
int conn_init(conn_t *, myuser_t *owner, const char *const *keys, const char *const *vals);


inline
int conn_init(conn_t *const c,
              myuser_t *const owner,
              const char *const *const keys,
              const char *const *const vals)
{
	memset(c, 0x0, sizeof(conn_t));
	c->creation = CURRTIME;
	c->owner = owner;
	c->connecting = true;

	if(!(c->conn = PQconnectStartParams(keys, vals, 1)))
	{
		pgerr = "Failed to start at all.";
		conn_fini(c);
		return 0;
	}

	if(!(c->pollable = mowgli_pollable_create(base_eventloop, PQsocket(c->conn), c)))
	{
		pgerr = "Failed to integrate into the eventloop.";
		conn_fini(c);
		return 0;
	}

	if(conn_status(c) == CONNECTION_BAD)
	{
		//pgerr = conn_errmsg(c);
		pgerr = "Failed due to malformed parameters.";
		conn_fini(c);
		return 0;
	}

	if(!PQsetnonblocking(c->conn, true) && !PQisnonblocking(c->conn))
		slog(LG_ERROR, "PGSQL: Could not set non-blocking.");

	if(!PQsetErrorVerbosity(c->conn, PQERRORS_VERBOSE))
		slog(LG_ERROR, "PGSQL: Could not set error verbosity.");

	if(!PQsetNoticeReceiver(c->conn, _conn_result_notice_handler, c))
		slog(LG_ERROR, "PGSQL: Could not register notice handler.");

	if(!PQregisterEventProc(c->conn, _conn_event_handler, "member_event_handler", c))
		slog(LG_ERROR, "PGSQL: Could not register event handler.");

	return 1;
}


inline
void conn_fini(conn_t *const c)
{
	if(!c)
		return;

	if(c->pollable)
		mowgli_pollable_destroy(base_eventloop, c->pollable);

	if(c->conn)
		PQfinish(c->conn);
}


inline
int conn_flush(conn_t *const c)
{
	return PQflush(c->conn);
}


inline
int conn_reset(conn_t *const c)
{
	c->connecting = true;

	if(c->pollable)
	{
		mowgli_pollable_destroy(base_eventloop, c->pollable);
		c->pollable = NULL;
	}

	if(!PQresetStart(c->conn))
	{
		pgerr = "Failed to start the reset";
		return 0;
	}

	if(!(c->pollable = mowgli_pollable_create(base_eventloop, PQsocket(c->conn), c)))
	{
		pgerr = "Failed to integrate into the eventloop.";
		return 0;
	}

	return 1;
}


inline
int conn_poll(conn_t *const c)
{
	return PQconnectPoll(c->conn);
}


inline
int conn_reset_poll(conn_t *const c)
{
	return PQresetPoll(c->conn);
}


inline
int conn_query(conn_t *const c,
               const char *const query)
{
	s_time(&c->query_tv);
	if(!PQsendQuery(c->conn, query))
		return 0;

	c->queries_sent++;
	return 1;
}


inline
int conn_query_cancel(conn_t *const c,
                      char *const errbuf,
                      const size_t bufmax)
{
	PGcancel *const cancel = PQgetCancel(c->conn);
	if(!PQcancel(cancel, errbuf, bufmax))
	{
		mowgli_strlcpy(errbuf, conn_errmsg(c), bufmax);
		PQfreeCancel(cancel);
		return 0;
	}

	PQfreeCancel(cancel);
	return 1;
}


inline
int conn_set_iocb(conn_t *const c,
                  const mowgli_eventloop_io_dir_t dir,
                  mowgli_eventloop_io_cb_t *const iocb)
{
	if(!c->pollable)
	{
		pgerr = "Connection has no pollable object!";
		return 0;
	}

	mowgli_pollable_setselect(base_eventloop, c->pollable, dir, iocb);
	return 1;
}


void _conn_result_notice_handler(void *const conn,
                                 const PGresult *const result)
{
	conn_t *const c = (conn_t *)conn;
	const char *const msg = PQresultErrorMessage(result);
	slog(LG_ERROR, "PGSQL: #%d: RESULT NOTICE: %s",
	               conn_id(c),
	               msg);

	if(c->handle_notice)
		c->handle_notice(c, result, msg);
}


int _conn_event_handler(PGEventId id,
                        void *const data,
                        void *const conn)
{
	conn_t *const c = (conn_t *)conn;
	slog(LG_DEBUG, "PGSQL: #%d: EVENT: %s (%d)",
	               conn_id(c),
	               reflect_event(id),
	               id);
	int ret = true;
	switch(id)
	{
		case PGEVT_REGISTER: if(c->handle_event_register)
			ret = c->handle_event_register(c, (PGEventRegister *)data);
			break;

		case PGEVT_CONNRESET: if(c->handle_event_conn_reset)
			ret = c->handle_event_conn_reset(c, (PGEventConnReset*)data);
			break;

		case PGEVT_CONNDESTROY: if(c->handle_event_conn_destroy)
			ret = c->handle_event_conn_destroy(c, (PGEventConnDestroy*)data);
			break;

		case PGEVT_RESULTCOPY: if(c->handle_event_result_copy)
			ret = c->handle_event_result_copy(c, (PGEventResultCopy*)data);
			break;

		case PGEVT_RESULTCREATE: if(c->handle_event_result_create)
			ret = c->handle_event_result_create(c, (PGEventResultCreate*)data);
			break;

		case PGEVT_RESULTDESTROY: if(c->handle_event_result_destroy)
			ret = c->handle_event_result_destroy(c, (PGEventResultDestroy*)data);
			break;
	}

	if(c->handle_event_any)
		c->handle_event_any(c, id, data);

	return ret;
}


inline
size_t conn_process(conn_t *const c)
{
	size_t ret = 0;
	conn_consume(c);
	if(conn_busy(c))
	{
		pgerr = "Connection is marked busy.";
		return ret;
	}

	ret += conn_process_result(c);
	ret += conn_process_notify(c);
	return ret;
}


inline
ssize_t conn_process_result(conn_t *const c)
{
	size_t ret = 0;
	PGresult *r = conn_get_result(c);
	if(r)
		e_time(c->query_tv, &c->query_tv);

	for(; r; r = conn_get_result(c), ++ret)
	{
		if(c->handle_result)
			c->handle_result(c, r);

		PQclear(r);
	}

	if(!c->handle_result && ret)
	{
		slog(LG_INFO, "PGSQL: #%d: Missed %zu results.", conn_id(c), ret);
		conn_tty(c, "Missed %zu results.", ret);
	}

	return ret;
}


inline
ssize_t conn_process_notify(conn_t *const c)
{
	size_t ret = 0;
	PGnotify *n = conn_get_notify(c);
	while(n)
	{
		if(c->handle_notify)
			c->handle_notify(c, n->relname, n->be_pid, n->extra);

		PGnotify *const last = n;
		n = n->next;
		PQfreemem(last);
		c->notifies_recv++;
		ret++;
	}

	if(!c->handle_notify && ret)
	{
		slog(LG_INFO, "PGSQL: #%d: Missed %zu notifies.", conn_id(c), ret);
		conn_tty(c, "Missed %zu notifies.", ret);
	}

	return ret;
}


inline
PGnotify *conn_get_notify(conn_t *const c)
{
	PGnotify *const ret = PQnotifies(c->conn);
	return ret;
}

inline
PGresult *conn_get_result(conn_t *const c)
{
	PGresult *const ret = PQgetResult(c->conn);
	if(!ret)
		return NULL;

	c->results_recv++;
	return ret;
}


inline
int conn_consume(conn_t *const c)
{
	return PQconsumeInput(c->conn);
}


inline
int conn_tty(conn_t *const c,
             const char *const fmt,
             ...)
{
	if(!c->tty && !c->owner)
		return 0;

	if(!c->tty && !c->owner->logins.count)
		return 0;

	va_list ap;
	va_start(ap, fmt);
	char buf[BUFSIZE];
	vsnprintf(buf, sizeof(buf), fmt, ap);

	if(c->tty)
		notice(myservice->me->nick, c->tty->nick, "%s", buf);
	else if(c->owner)
		myuser_notice(myservice->me->nick, c->owner, "#%d: %s", conn_id(c), buf);

	va_end(ap);
	return 1;
}


inline
const char *conn_errmsg(const conn_t *const c)
{
	if(!c->conn)
		return "Connection is NUL.";

	const char *const ret = PQerrorMessage(c->conn);
	if(!ret)
		return "No error message.";

	return ret;
}


inline
PGTransactionStatusType conn_trans_status(const conn_t *const c)
{
	return PQtransactionStatus(c->conn);
}


inline
ConnStatusType conn_status(const conn_t *const c)
{
	return PQstatus(c->conn);
}


inline
int conn_busy(const conn_t *const c)
{
	return PQisBusy(c->conn);
}


inline
int conn_id(const conn_t *const c)
{
	return c->id;
}
