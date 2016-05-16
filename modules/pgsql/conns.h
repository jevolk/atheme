/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */

typedef struct
{
	mowgli_list_t list;         // state
	uint64_t cnt;               // monotonically increasing counter
}
conns_t;

void conns_foreach(const conns_t *conns, void (*func)(const conn_t *));
void conns_foreach_mutable(conns_t *conns, void (*func)(conn_t *));
int conns_until(const conns_t *conns, int (*func)(const conn_t *));
int conns_until_mutable(conns_t *conns, int (*func)(conn_t *));
size_t conns_count(const conns_t *conns);
const conn_t *conns_find_by_id(const conns_t *conns, const int id);
conn_t *conns_find_by_id_mutable(conns_t *conns, const int id);
int conns_del(conns_t *conns, conn_t *conn);
int conns_add(conns_t *conns, conn_t *conn);


inline
int conns_add(conns_t *const conns,
              conn_t *const conn)
{
	mowgli_node_t *node = mowgli_node_create();
	mowgli_node_add(conn, node, &conns->list);
	conn->id = ++conns->cnt;
	return 1;
}


inline
int conns_del(conns_t *const conns,
              conn_t *const conn)
{
	mowgli_node_t *node = mowgli_node_find(conn, &conns->list);
	if(!node)
		return 0;

	mowgli_node_delete(node, &conns->list);
	mowgli_node_free(node);
	return 1;
}


inline
conn_t *conns_find_by_id_mutable(conns_t *const conns,
                                 const int id)
{
	conn_t *ret = NULL;
	int match(conn_t *const c)
	{
		if(conn_id(c) != id)
			return 0;

		ret = c;
		return 1;
	}

	conns_until_mutable(conns, match);
	return ret;
}


inline
const conn_t *conns_find_by_id(const conns_t *const conns,
                               const int id)
{
	const conn_t *ret = NULL;
	int match(const conn_t *const c)
	{
		if(conn_id(c) != id)
			return 0;

		ret = c;
		return 1;
	}

	conns_until(conns, match);
	return ret;
}


inline
size_t conns_count(const conns_t *const conns)
{
	return MOWGLI_LIST_LENGTH(&conns->list);
}


inline
int conns_until_mutable(conns_t *const conns,
                        int (*const func)(conn_t *))
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, conns->list.head)
	{
		conn_t *const conn = n->data;
		if(func(conn))
			return 1;
	}

	return 0;
}


inline
int conns_until(const conns_t *const conns,
                int (*const func)(const conn_t *))
{
	mowgli_node_t *n;
	MOWGLI_LIST_FOREACH(n, (mowgli_node_t *)conns->list.head)
	{
		const conn_t *const conn = n->data;
		if(func(conn))
			return 1;
	}

	return 0;
}


inline
void conns_foreach_mutable(conns_t *const conns,
                           void (*const func)(conn_t *))
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, conns->list.head)
	{
		conn_t *const conn = n->data;
		func(conn);
	}
}


inline
void conns_foreach(const conns_t *const conns,
                   void (*const func)(const conn_t *))
{
	mowgli_node_t *n;
	MOWGLI_LIST_FOREACH(n, (mowgli_node_t *)conns->list.head)
	{
		const conn_t *const conn = n->data;
		func(conn);
	}
}
