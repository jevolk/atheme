/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */

typedef struct
{
	mowgli_list_t list;       // state (will become dict eventually)
}
sesss_t;

void sesss_foreach(const sesss_t *sesss, void (*func)(const sess_t *));
void sesss_foreach_mutable(sesss_t *sesss, void (*func)(sess_t *));
int sesss_until(const sesss_t *sesss, int (*func)(const sess_t *));
int sesss_until_mutable(sesss_t *sesss, int (*func)(sess_t *));
size_t sesss_count(const sesss_t *sesss);
const sess_t *sesss_find(const sesss_t *sesss, const user_t *);
sess_t *sesss_find_mutable(sesss_t *sesss, const user_t *);
int sesss_del(sesss_t *sesss, sess_t *sess);
int sesss_add(sesss_t *sesss, sess_t *sess);


inline
int sesss_add(sesss_t *const sesss,
              sess_t *const sess)
{
	mowgli_node_t *node = mowgli_node_create();
	mowgli_node_add(sess, node, &sesss->list);
	return 1;
}


inline
int sesss_del(sesss_t *const sesss,
              sess_t *const sess)
{
	mowgli_node_t *node = mowgli_node_find(sess, &sesss->list);
	if(!node)
		return 0;

	mowgli_node_delete(node, &sesss->list);
	mowgli_node_free(node);
	return 1;
}


inline
sess_t *sesss_find_mutable(sesss_t *const sesss,
                           const user_t *const user)
{
	sess_t *ret = NULL;
	int match(sess_t *const s)
	{
		if(s->user != user)
			return 0;

		ret = s;
		return 1;
	}

	sesss_until_mutable(sesss, match);
	return ret;
}


inline
const sess_t *sesss_find(const sesss_t *const sesss,
                         const user_t *const user)
{
	const sess_t *ret = NULL;
	int match(const sess_t *const s)
	{
		if(s->user != user)
			return 0;

		ret = s;
		return 1;
	}

	sesss_until(sesss, match);
	return ret;
}


inline
size_t sesss_count(const sesss_t *const sesss)
{
	return MOWGLI_LIST_LENGTH(&sesss->list);
}


inline
int sesss_until_mutable(sesss_t *const sesss,
                        int (*const func)(sess_t *))
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, sesss->list.head)
	{
		sess_t *const sess = n->data;
		if(func(sess))
			return 1;
	}

	return 0;
}


inline
int sesss_until(const sesss_t *const sesss,
                int (*const func)(const sess_t *))
{
	mowgli_node_t *n;
	MOWGLI_LIST_FOREACH(n, (mowgli_node_t *)sesss->list.head)
	{
		const sess_t *const sess = n->data;
		if(func(sess))
			return 1;
	}

	return 0;
}


inline
void sesss_foreach_mutable(sesss_t *const sesss,
                           void (*const func)(sess_t *))
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, sesss->list.head)
	{
		sess_t *const sess = n->data;
		func(sess);
	}
}


inline
void sesss_foreach(const sesss_t *const sesss,
                   void (*const func)(const sess_t *))
{
	mowgli_node_t *n;
	MOWGLI_LIST_FOREACH(n, (mowgli_node_t *)sesss->list.head)
	{
		const sess_t *const sess = n->data;
		func(sess);
	}
}
