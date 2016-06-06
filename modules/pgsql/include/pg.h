/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations, idgaf:
 * - Atheme Development Group
 * - Freenode
 */

typedef struct
{
	service_t *svc;                          // Atheme service tag
	conns_t conns;                           // Container of database connections (conn_t's)
	sesss_t sesss;                           // Container of sessions (sess_t's)
	mowgli_eventloop_io_cb_t *handler;       // Main event dispatcher location (pgsql.c)
}
PG;

int pg_query2(PG *, conn_t *, const char *query, conn_handle_result_t handler);                   // Runs the query, updating and using the handler
int pg_query(PG *, conn_t *, const char *query);
int pg_ping(PG *, const char *const *keys, const char *const *vals, const char **ret);            // [WARNING: BLOCKS] assigns static string if ret!=NULL

#ifdef PG_MODULE_INTERNAL
const sess_t *pg_sess_find(const PG *, const user_t *user);
sess_t *pg_sess_find_mutable(PG *, const user_t *user);
void pg_sess_delete(PG *, sess_t *);
size_t pg_sesss_delete_if(PG *, int (*func)(sess_t *));
size_t pg_sesss_delete(PG *);                                                                     // terminate all sessions
sess_t *pg_sess_new(PG *, user_t *user, conn_t *conn);
#endif

const conn_t *pg_conn_find(const PG *, const int id);
conn_t *pg_conn_find_mutable(PG *, const int id);
void pg_conn_delete(PG *, conn_t *);
size_t pg_conns_delete_if(PG *, int (*func)(conn_t *));
size_t pg_conns_delete(PG *);                                                                     // terminate all connections
int pg_conn_reset(PG *, conn_t *);
conn_t *pg_conn_new(PG *, myuser_t *owner, const char *const *keys, const char *const *vals);


inline
conn_t *pg_conn_new(PG *const pg,
                    myuser_t *owner,
                    const char *const *const keys,
                    const char *const *const vals)
{
	conn_t *const c = (conn_t *)mowgli_alloc(sizeof(conn_t));
	if(!conn_init(c, owner, keys, vals))
	{
		mowgli_free(c);
		return NULL;
	}

	if(!conns_add(&pg->conns, c))
	{
		pg_conn_delete(pg, c);
		return NULL;
	}

	if(!conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_READ, pg->handler) ||
	   !conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_WRITE, pg->handler))
	{
		pg_conn_delete(pg, c);
		return NULL;
	}

	return c;
}


inline
int pg_conn_reset(PG *const pg,
                  conn_t *const c)
{
	if(!conn_reset(c))
		return 0;

	if(!conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_READ, pg->handler) ||
	   !conn_set_iocb(c, MOWGLI_EVENTLOOP_IO_WRITE, pg->handler))
	{
		pg_conn_delete(pg, c);
		return 0;
	}

	return 1;
}


#ifdef PG_MODULE_INTERNAL
inline
size_t pg_conns_delete(PG *const pg)
{
	int deleter(conn_t *const conn)
	{
		return true;
	}

	return pg_conns_delete_if(pg, deleter);
}
#endif


#ifndef __cplusplus
inline
size_t pg_conns_delete_if(PG *const pg,
                          int (*const func)(conn_t *))
{
	size_t ret = 0;
	void deleter(conn_t *const conn)
	{
		if(func(conn))
		{
			pg_conn_delete(pg, conn);
			ret++;
		}
	}

	conns_foreach_mutable(&pg->conns, deleter);
	return ret;
}
#endif


inline
void pg_conn_delete(PG *const pg,
                    conn_t *const c)
{
	if(!c)
		return;

	#ifdef PG_MODULE_INTERNAL
	int has_conn(sess_t *const s)
	{
		return s->conn == c;
	}

	pg_sesss_delete_if(pg, has_conn);
	#endif

	conns_del(&pg->conns, c);
	conn_fini(c);
	mowgli_free(c);
}


#ifdef PG_MODULE_INTERNAL
inline
sess_t *pg_sess_new(PG *const pg,
                    user_t *const user,
                    conn_t *const conn)
{
	sess_t *const s = (sess_t *)mowgli_alloc(sizeof(sess_t));
	if(!sess_init(s, user, conn))
	{
		mowgli_free(s);
		return NULL;
	}

	if(!sesss_add(&pg->sesss, s))
	{
		pg_sess_delete(pg, s);
		return NULL;
	}

	return s;
}
#endif


#ifdef PG_MODULE_INTERNAL
inline
size_t pg_sesss_delete(PG *const pg)
{
	int deleter(sess_t *const sess)
	{
		return true;
	}

	return pg_sesss_delete_if(pg, deleter);
}
#endif


#ifdef PG_MODULE_INTERNAL
inline
size_t pg_sesss_delete_if(PG *const pg,
                          int (*const func)(sess_t *))
{
	size_t ret = 0;
	void deleter(sess_t *const sess)
	{
		if(func(sess))
		{
			pg_sess_delete(pg, sess);
			ret++;
		}
	}

	sesss_foreach_mutable(&pg->sesss, deleter);
	return ret;
}
#endif


#ifdef PG_MODULE_INTERNAL
inline
void pg_sess_delete(PG *const pg,
                    sess_t *const s)
{
	if(!s)
		return;

	sesss_del(&pg->sesss, s);
	sess_fini(s);
	mowgli_free(s);
}
#endif


#ifdef PG_MODULE_INTERNAL
inline
sess_t *pg_sess_find_mutable(PG *const pg,
                             const user_t *const user)
{
	sess_t *const s = sesss_find_mutable(&pg->sesss, user);
	if(!s)
	{
		pgerr = "No session for that user";
		return NULL;
	}

	return s;
}
#endif


#ifdef PG_MODULE_INTERNAL
inline
const sess_t *pg_sess_find(const PG *const pg,
                           const user_t *user)
{
	const sess_t *const s = sesss_find(&pg->sesss, user);
	if(!s)
	{
		pgerr = "No session for that user";
		return NULL;
	}

	return s;
}
#endif


inline
int pg_ping(PG *const pg,
            const char *const *const keys,
            const char *const *const vals,
            const char **retstr)
{
	PGPing ret = PQpingParams(keys, vals, 1);
	pgerr = reflect_ping(ret);
	if(retstr)
		*retstr = reflect_ping(ret);

	return ret;
}


inline
int pg_query(PG *const pg,
             conn_t *const c,
             const char *const query)
{
	if(!conn_query(c, query))
		return 0;

	return 1;
}


inline
int pg_query2(PG *const pg,
              conn_t *const c,
              const char *const query,
              conn_handle_result_t handler)
{
	c->handle_result = handler;
	return conn_query(c, query);
}


inline
conn_t *pg_conn_find_mutable(PG *const pg,
                             const int id)
{
	if(!id)
	{
		pgerr = "Bad ID number.";
		return NULL;
	}

	conn_t *const c = conns_find_by_id_mutable(&pg->conns, id);
	if(!c)
	{
		pgerr = "No connection with that ID";
		return NULL;
	}

	return c;
}


inline
const conn_t *pg_conn_find(const PG *const pg,
                           const int id)
{
	if(!id)
	{
		pgerr = "Bad ID number.";
		return NULL;
	}

	const conn_t *const c = conns_find_by_id(&pg->conns, id);
	if(!c)
	{
		pgerr = "No connection with that ID";
		return NULL;
	}

	return c;
}
