/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef struct
{
	mowgli_patricia_t *groups;
}
groups_t;

void groups_foreach(const groups_t *groups, void (*func)(const group_t *));
int groups_until(const groups_t *groups, int (*func)(const group_t *));
void groups_foreach_mutable(groups_t *groups, void (*func)(group_t *));
int groups_until_mutable(groups_t *groups, int (*func)(group_t *));
size_t groups_count(const groups_t *groups);

void groups_elem_foreach(const groups_t *groups, const group_attr_t attr, void (*func)(const group_t *, const void *));
void groups_elem_foreach_mutable(groups_t *groups, const group_attr_t attr, void (*func)(group_t *, void *));
const void *groups_elem_find(const groups_t *groups, const group_attr_t attr, int (*func)(const group_t *, const void *));
void *groups_elem_find_mutable(groups_t *groups, const group_attr_t attr, int (*func)(group_t *, void *));
size_t groups_elem_count_if(const groups_t *groups, const group_attr_t attr, int (*func)(const group_t *, const void *));
size_t groups_count_elems(const groups_t *groups, const group_attr_t attr);

void groups_user_foreach_mutable(groups_t *groups, const myuser_t *user, void (*func)(group_t *));
void groups_user_foreach(const groups_t *groups, const myuser_t *user, void (*func)(const group_t *));
int groups_user_until_mutable(groups_t *groups, const myuser_t *user, int (*func)(group_t *));
int groups_user_until(const groups_t *groups, const myuser_t *user, int (*func)(const group_t *));
size_t groups_user_count(const myuser_t *user);

const group_t *groups_find_if(const groups_t *groups, int (*func)(const group_t *));
group_t *groups_find_if_mutable(groups_t *groups, int (*func)(const group_t *));
const group_t *groups_find(const groups_t *groups, const char *name);
group_t *groups_find_mutable(groups_t *groups, const char *name);
const group_t *groups_find_by_channame(const groups_t *groups, const char *name);
group_t *groups_find_by_channame_mutable(groups_t *groups, const char *name);

int groups_del(groups_t *groups, group_t *group);
int groups_add(groups_t *groups, group_t *group);

void groups_write(const groups_t *groups, database_handle_t *const db);
void groups_read(groups_t *groups, database_handle_t *const db, const char *const type);

void groups_fini(groups_t *groups);
void groups_init(groups_t *groups);


inline
void groups_init(groups_t *const groups)
{
	groups->groups = mowgli_patricia_create(irccasecanon);
}


inline
void groups_fini(groups_t *const groups)
{
	void fini_group(const char *const key, void *const elem, void *const priv)
	{
		group_t *const group = elem;
		group_fini(group);
	}

	mowgli_patricia_destroy(groups->groups, fini_group, NULL);
}


inline
void groups_read(groups_t *const groups,
                 database_handle_t *const db,
                 const char *const type)
{
	const char *const name = db_sread_word(db);
	group_t *group = groups_find_mutable(groups, name);

	if(!group)
	{
		group = mowgli_alloc(sizeof(group_t));
		group_init(group);
		group_name_set(group, name);
		groups_add(groups, group);
	}

	group_read(group, db, type);
}


inline
void groups_write(const groups_t *const groups,
                  database_handle_t *const db)
{
	void write_group(const group_t *const group)
	{
		group_write(group, db);
	}

	groups_foreach(groups, write_group);
}


inline
int groups_add(groups_t *const groups,
               group_t *const group)
{
	const int ret = mowgli_patricia_add(groups->groups, group->name, group);
	return ret;
}


inline
int groups_del(groups_t *const groups,
               group_t *const group)
{
	void *ret = mowgli_patricia_delete(groups->groups, group->name);
	return ret != NULL;
}


inline
group_t *groups_find_by_channame_mutable(groups_t *const groups,
                                         const char *const name)
{
	if(strlen(name) <= 1 || name[0] != '#')
		return NULL;

	int prefix_match(const group_t *const group)
	{
		const size_t len = strlen(group->name);
		return ircncasecmp(group->name, name+1, len) == 0;
	}

	return groups_find_if_mutable(groups, prefix_match);
}


inline
const group_t *groups_find_by_channame(const groups_t *const groups,
                                       const char *const name)
{
	if(strlen(name) <= 1 || name[0] != '#')
		return NULL;

	int prefix_match(const group_t *const group)
	{
		const size_t len = strlen(group->name);
		return ircncasecmp(group->name, name+1, len) == 0;
	}

	return groups_find_if(groups, prefix_match);
}


inline
group_t *groups_find_mutable(groups_t *const groups,
                             const char *const name)
{
	void *const data = mowgli_patricia_retrieve(groups->groups, name);
	return (group_t *)data;
}


inline
const group_t *groups_find(const groups_t *const groups,
                           const char *const name)
{
	void *const data = mowgli_patricia_retrieve(groups->groups, name);
	return (const group_t *)data;
}


inline
group_t *groups_find_if_mutable(groups_t *const groups,
                                int (*const func)(const group_t *))
{
	group_t *ret = NULL;
	int match(group_t *const group)
	{
		if(func(group))
		{
			ret = group;
			return 1;
		}

		return 0;
	}

	groups_until_mutable(groups, match);
	return ret;
}


inline
const group_t *groups_find_if(const groups_t *const groups,
                              int (*const func)(const group_t *))
{
	const group_t *ret = NULL;
	int match(const group_t *const group)
	{
		if(func(group))
		{
			ret = group;
			return 1;
		}

		return 0;
	}

	groups_until(groups, match);
	return ret;
}


inline
size_t groups_user_count(const myuser_t *const user)
{
	return user_meta_list_count(user, "groups");
}


inline
int groups_user_until_mutable(groups_t *const groups,
                              const myuser_t *const user,
                              int (*const func)(group_t *))
{
	int name_to_group(const char *const name)
	{
		group_t *const group = groups_find_mutable(groups, name);
		return group? func(group) : 0;
	}

	return user_meta_list_until(user, "groups", name_to_group);
}


inline
int groups_user_until(const groups_t *const groups,
                      const myuser_t *const user,
                      int (*const func)(const group_t *))
{
	int name_to_group(const char *const name)
	{
		const group_t *const group = groups_find(groups, name);
		return group? func(group) : 0;
	}

	return user_meta_list_until(user, "groups", name_to_group);
}


inline
void groups_user_foreach(const groups_t *const groups,
                         const myuser_t *const user,
                         void (*const func)(const group_t *))
{
	void name_to_group(const char *const name)
	{
		const group_t *const group = groups_find(groups, name);
		if(group)
			func(group);
	}

	user_meta_list_foreach(user, "groups", name_to_group);
}


inline
void groups_user_foreach_mutable(groups_t *const groups,
                                 const myuser_t *const user,
                                 void (*const func)(group_t *))
{
	void name_to_group(const char *const name)
	{
		group_t *const group = groups_find_mutable(groups, name);
		if(group)
			func(group);
	}

	user_meta_list_foreach(user, "groups", name_to_group);
}


inline
size_t groups_count_elems(const groups_t *groups,
                          const group_attr_t attr)
{
	size_t ret = 0;
	void counter(const group_t *const group)
	{
		ret += group_count(group, attr);
	}

	groups_foreach(groups, counter);
	return ret;
}


inline
size_t groups_elem_count_if(const groups_t *groups,
                            const group_attr_t attr,
                            int (*const func)(const group_t *, const void *))
{
	size_t ret = 0;
	void count(const group_t *const group, const void *const elem)
	{
		if(func(group, elem))
			ret++;
	}

	groups_elem_foreach(groups, attr, count);
	return ret;
}


inline
void *groups_elem_find_mutable(groups_t *const groups,
                               const group_attr_t attr,
                               int (*const func)(group_t *, void *))
{
	void *ret = NULL;
	int until_group(group_t *const group)
	{
		int until_elem(void *const elem)
		{
			if(func(group, elem))
			{
				ret = elem;
				return 1;
			}
		}

		return group_until_mutable(group, attr, until_elem);
	}

	groups_until_mutable(groups, until_group);
	return ret;
}


inline
const void *groups_elem_find(const groups_t *const groups,
                             const group_attr_t attr,
                             int (*const func)(const group_t *, const void *))
{
	const void *ret = NULL;
	int until_group(const group_t *const group)
	{
		int until_elem(const void *const elem)
		{
			if(func(group, elem))
			{
				ret = elem;
				return 1;
			}
		}

		return group_until(group, attr, until_elem);
	}

	groups_until(groups, until_group);
	return ret;
}


inline
void groups_elem_foreach_mutable(groups_t *const groups,
                                 const group_attr_t attr,
                                 void (*const func)(group_t *, void *))
{
	void foreach_group(group_t *const group)
	{
		void foreach_elem(void *const elem)
		{
			func(group, elem);
		}

		group_foreach_mutable(group, attr, foreach_elem);
	}

	groups_foreach_mutable(groups, foreach_group);
}


inline
void groups_elem_foreach(const groups_t *const groups,
                         const group_attr_t attr,
                         void (*const func)(const group_t *, const void *))
{
	void foreach_group(const group_t *const group)
	{
		void foreach_elem(const void *const elem)
		{
			func(group, elem);
		}

		group_foreach(group, attr, foreach_elem);
	}

	groups_foreach(groups, foreach_group);
}


inline
size_t groups_count(const groups_t *groups)
{
	return mowgli_patricia_size(((groups_t *)groups)->groups);
}


inline
int groups_until_mutable(groups_t *const groups,
                         int (*const func)(group_t *))
{
	group_t *group;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(group, &state, groups->groups)
		if(func(group))
			return 1;

	return 0;
}


inline
void groups_foreach_mutable(groups_t *const groups,
                            void (*const func)(group_t *))
{
	group_t *group;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(group, &state, groups->groups)
		func(group);
}


inline
int groups_until(const groups_t *const groups,
                 int (*const func)(const group_t *))
{
	const group_t *group;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(group, &state, ((groups_t *)groups)->groups)
		if(func(group))
			return 1;

	return 0;
}


inline
void groups_foreach(const groups_t *const groups,
                    void (*const func)(const group_t *))
{
	const group_t *group;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(group, &state, ((groups_t *)groups)->groups)
		func(group);
}
