/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#define GMS_DBKEY_GROUP          GMS_NAME "G"
#define GMS_DBKEY_GROUP_META     GMS_NAME "M"

typedef enum
{
	GROUP_USER,         // myuser_t *
	GROUP_CHAN,         // mychan_t *
	GROUP_ACCESS,       // access_t *
	GROUP_REQUEST,      // request_t *

	_GROUP_ATTRS_
}
group_attr_t;

typedef mowgli_index_t group_attrs_t;

size_t gms_dbkey_group(const group_attr_t attr, char *buf);

typedef struct
{
	void (*write)(database_handle_t *db, const void *elem);
	int (*read)(database_handle_t *db, const char *type, void **elem);
	void (*dtor)(void *elem);
}
group_vtable_t;

typedef struct
{
	meta_t meta;
	group_attrs_t attr[_GROUP_ATTRS_];
	char name[32];
	time_t creation;
	uint mode;
}
group_t;

const group_vtable_t *group_vtable;

// Abstract container interface
const group_attrs_t *group_attrs(const group_t *const group, const group_attr_t attr);
group_attrs_t *group_attrs_mutable(group_t *const group, const group_attr_t attr);
void group_foreach(const group_t *group, const group_attr_t attr, void (*func)(const void *));
void group_foreach_mutable(group_t *group, const group_attr_t attr, void (*func)(void *));
int group_until(const group_t *group, const group_attr_t attr, int (*func)(const void *));
int group_until_mutable(group_t *group, const group_attr_t attr, int (*func)(void *));
void group_remove_if(group_t *group, const group_attr_t attr, int (*func)(void *));
size_t group_index(const group_t *group, const group_attr_t attr, int (*func)(const void *));
size_t group_pos(const group_t *group, const group_attr_t attr, const void *elem);
const void *group_find(const group_t *group, const group_attr_t attr, int (*func)(const void *));
void *group_find_mutable(group_t *group, const group_attr_t attr, int (*func)(const void *));
const void *group_at(const group_t *group, const group_attr_t attr, const size_t idx);
void *group_at_mutable(group_t *group, const group_attr_t attr, const size_t idx);
size_t group_count(const group_t *group, const group_attr_t attr);
int group_has(const group_t *group, const group_attr_t attr, const void *ent);
int group_del(group_t *group, const group_attr_t attr, const void *ent);
int group_add(group_t *group, const group_attr_t attr, void *ent);

// meta interface
void group_meta_foreach(const group_t *group, void (*func)(const char *, const char *));
int group_meta_until(const group_t *group, int (*func)(const char *, const char *));
size_t group_meta_count(const group_t *group);
const char *group_meta_get(const group_t *group, const char *key);
void group_meta_set(group_t *group, const char *key, const char *value);
int group_meta_del(group_t *group, const char *key);

// misc interface
int group_name_valid(const char *const name);
const char *group_name(const group_t *group);
size_t group_name_set(group_t *group, const char *name);
time_t group_created(const group_t *const group);
uint group_mode(const group_t *const group);
uint group_mode_delta(group_t *const group, const char *const flags);

// GROUP_ACCESS convenience interface
const access_t *group_access_find(const group_t *group, const myuser_t *user);
access_t *group_access_find_mutable(group_t *group, const myuser_t *user);
int group_access_del(group_t *group, const access_t *access);
int group_access_add(group_t *group, access_t *access);
const char *group_access_role(const group_t *group, const myuser_t *user);
const char *group_access_cloak(const group_t *group, const myuser_t *user);
int group_access_anyof(const group_t *group, const myuser_t *user, const access_flag_t mask);
int group_access_check(const group_t *group, const myuser_t *user, const access_flag_t mask);
int group_access_delta(group_t *const group, myuser_t *const user, const char *const flags);

// GROUP_CHAN convenience interface
mychan_t *group_chan_main(group_t *group);
void group_chan_acquire(group_t *group, mychan_t *const chan);
void group_chan_clear(group_t *group);

// GROUP_USER convenience interface
int group_user_has(const group_t *group, const myuser_t *user);
int group_user_add(group_t *group, myuser_t *user);
int group_user_del(group_t *group, myuser_t *user);
void group_user_clear(group_t *group);

// group_t serial
void group_write(const group_t *group, database_handle_t *db);
void group_read(group_t *group, database_handle_t *db, const char *type);

// group_t
void group_fini(group_t *group);
void group_init(group_t *group);


inline
void group_vtable_init(group_vtable_t *const vtable)
{
	memset(vtable, 0x0, sizeof(group_vtable_t));
	vtable[GROUP_ACCESS].read = access_read;
	vtable[GROUP_ACCESS].write = access_write;
	vtable[GROUP_ACCESS].dtor = access_delete;
	vtable[GROUP_REQUEST].read = request_read;
	vtable[GROUP_REQUEST].write = request_write;
	vtable[GROUP_REQUEST].dtor = request_delete;
}


inline
void group_init(group_t *const group)
{
	memset(group, 0x0, sizeof(group_t));
	meta_init(&group->meta, GMS_DBKEY_GROUP_META);
	for(size_t i = 0; i < _GROUP_ATTRS_; i++)
		mowgli_index_init(group->attr + i);
}


inline
void group_fini(group_t *const group)
{
	for(size_t i = 0; i < _GROUP_ATTRS_; i++)
	{
		if(group_vtable[i].dtor)
		{
			void dtor(void *const elem)
			{
				group_vtable[i].dtor(elem);
			}

			group_foreach_mutable(group, i, dtor);
		}

		mowgli_index_fini(group->attr + i);
	}

	meta_fini(&group->meta);
}


inline
void group_read(group_t *const group,
                database_handle_t *const db,
                const char *const type)
{
	if(strcmp(GMS_DBKEY_GROUP, type) == 0)
	{
		group->creation = db_sread_time(db);
		group->mode = mode_str_to_mask(db_sread_word(db));
		return;
	}

	const uint attr = atoi(type + strlen(GMS_DBKEY_GROUP));
	if(attr >= _GROUP_ATTRS_)
	{
		slog(LG_ERROR, "group_read(): type[%s] attr[%u]: attr out of range", type, attr);
		return;
	}

	const group_vtable_t *const vtable = group_vtable + attr;
	if(!vtable->read)
	{
		slog(LG_DEBUG, "group_read(): type[%s] attr[%u]: no read handler", type, attr);
		return;
	}

	void *elem;
	if(!vtable->read(db, type, &elem))
	{
		slog(LG_ERROR, "group_read(): type[%s] attr[%u]: read handler: %s", type, attr, gmserr);
		return;
	}

	group_add(group, attr, elem);
}


inline
void group_write(const group_t *const group,
                 database_handle_t *const db)
{
	char mode_buf[32];
	mode_mask_to_str(group->mode, mode_buf);

	db_start_row(db, GMS_DBKEY_GROUP);
	db_write_word(db, group_name(group));
	db_write_time(db, group->creation);
	db_write_word(db, mode_buf);
	db_commit_row(db);

	for(size_t i = 0; i < _GROUP_ATTRS_; i++)
	{
		const group_vtable_t *const vtable = group_vtable + i;
		if(!vtable->write)
			continue;

		char buf[16];
		gms_dbkey_group(i, buf);
		void write_elem(const void *const elem)
		{
			if(!elem)
				return;

			db_start_row(db, buf);
			db_write_word(db, group_name(group));
			vtable->write(db, elem);
			db_commit_row(db);
		}

		group_foreach(group, i, write_elem);
    }
}


inline
void group_user_clear(group_t *const group)
{
	size_t count;
	while((count = group_count(group, GROUP_USER)))
	{
		myuser_t *const user = group_at_mutable(group, GROUP_USER, count - 1);
		group_user_del(group, user);
	}
}


inline
int group_user_del(group_t *const group,
                   myuser_t *const user)
{
	int access_for_user(void *const a)
	{
		access_t *const acc = a;
		if(acc->user == user)
		{
			access_delete(acc);
			return 1;
		}

		return 0;
	}

	user_meta_list_remove(user, "groups", group_name(group));
	group_remove_if(group, GROUP_ACCESS, access_for_user);
	group_del(group, GROUP_USER, user);
	return 1;
}


inline
int group_user_add(group_t *const group,
                   myuser_t *const user)
{
	if(group_user_has(group, user))
		return 0;

	user_meta_list_append(user, "groups", group_name(group));
	group_add(group, GROUP_USER, user);
	return 1;
}


inline
int group_user_has(const group_t *const group,
                   const myuser_t *const user)
{
	return user_meta_list_has(user, "groups", group_name(group));
}


inline
void group_chan_clear(group_t *const group)
{
	size_t count;
	while((count = group_count(group, GROUP_CHAN)))
	{
		mychan_t *const chan = group_at_mutable(group, GROUP_CHAN, count - 1);
		// hook_channel_drop(gms.c) performs the group_del(); if not, do it here.
		chan_drop(chan);
	}
}


inline
void group_chan_acquire(group_t *const group,
                        mychan_t *const chan)
{
	void add_chanacs(void *const access)
	{
		chan_access_sync(chan, access);
	}

	chan_clear_acs(chan);
	group_add(group, GROUP_CHAN, chan);
	group_foreach_mutable(group, GROUP_ACCESS, add_chanacs);
}


inline
mychan_t *chan_main(group_t *const group)
{
	return group_at_mutable(group, GROUP_CHAN, 0);
}


inline
int group_access_delta(group_t *const group,
                       myuser_t *const user,
                       const char *const flags)
{
	access_t *acc = group_access_find_mutable(group, user);

	// Adds a fresh entry if the user doesn't exist
	if(!acc)
	{
		// If the flags were all negative or missing, nothing is added.
		uint mask = 0;
		if(!mask_delta(&access_vtable, flags, &mask))
		{
			gmserr = "No flags specified for new entry.";
			return 0;
		}

		acc = mowgli_alloc(sizeof(access_t));
		access_init(acc, user, mask, NULL);
		group_access_add(group, acc);
		return 1;
	}

	// If the delta cleared all the flags from the user, remove the entry
	if(!access_update(acc, flags))
	{
		group_access_del(group, acc);
		mowgli_free(acc);
		return 1;
	}

	acc->modified = CURRTIME;
	return 1;
}


inline
int group_access_anyof(const group_t *const group,
                      const myuser_t *const user,
                      const access_flag_t mask)
{
	const access_t *const acc = group_access_find(group, user);
	return acc? (acc->mask & mask) : 0;
}


inline
int group_access_check(const group_t *const group,
                       const myuser_t *const user,
                       const access_flag_t mask)
{
	const access_t *const acc = group_access_find(group, user);
	return acc? (acc->mask & mask) == mask : 0;
}


inline
const char *group_access_cloak(const group_t *const group,
                               const myuser_t *const user)
{
	const access_t *const access = group_access_find(group, user);
	return access && access->mask & GA_CLOAK? access->role : NULL;
}


inline
const char *group_access_role(const group_t *const group,
                              const myuser_t *const user)
{
	const access_t *const access = group_access_find(group, user);
	return access? access->role : NULL;
}


inline
int group_access_add(group_t *const group,
                     access_t *const access)
{
	if(!access->user)
	{
		gmserr = "No user associated with access.";
		return 0;
	}

	if(group_access_find(group, access->user))
	{
		gmserr = "Already exists.";
		return 0;
	}

	group_add(group, GROUP_ACCESS, access);
	return 1;
}


inline
int group_access_del(group_t *const group,
                     const access_t *const access)
{
	return group_del(group, GROUP_ACCESS, access);
}


inline
access_t *group_access_find_mutable(group_t *const group,
                                    const myuser_t *const user)
{
	int match(const void *const a)
	{
		const access_t *const acc = a;
		return acc->user == user;
	}

	return group_find_mutable(group, GROUP_ACCESS, match);
}


inline
const access_t *group_access_find(const group_t *const group,
                                  const myuser_t *const user)
{
	int match(const void *const a)
	{
		const access_t *const acc = a;
		return acc->user == user;
	}

	return group_find(group, GROUP_ACCESS, match);
}


inline
int group_meta_del(group_t *const group,
                   const char *const key)
{
	return meta_del(&group->meta, key);
}


inline
void group_meta_set(group_t *const group,
                    const char *const key,
                    const char *const value)
{
	meta_set(&group->meta, key, value);
}


inline
const char *group_meta_get(const group_t *const group,
                           const char *const key)
{
	return meta_get(&group->meta, key);
}


inline
size_t group_meta_count(const group_t *const group)
{
	size_t ret = 0;
	void counter(const char *const k, const char *const v)
	{
		ret++;
	}

	group_meta_foreach(group, counter);
	return ret;
}


inline
int group_meta_until(const group_t *const group,
                     int (*func)(const char *, const char *))
{
	return metadata_until(object(group), func);
}


inline
void group_meta_foreach(const group_t *const group,
                        void (*func)(const char *, const char *))
{
	metadata_foreach(object(group), func);
}


inline
uint group_mode_delta(group_t *const group,
                      const char *const flags)
{
	return mode_mask_delta(flags, &group->mode);
}


inline
uint group_mode(const group_t *const group)
{
	return group->mode;
}


inline
time_t group_created(const group_t *const group)
{
	return group->creation;
}


inline
size_t group_name_set(group_t *const group,
                      const char *const name)
{
	return mowgli_strlcpy(group->name, name, sizeof(group->name));
}


inline
const char *group_name(const group_t *const group)
{
	return group->name;
}


inline
int group_name_valid(const char *const name)
{
	if(!name || !strlen(name))
		return 0;

	return 1;
}


inline
int group_del(group_t *const group,
              const group_attr_t attr,
              const void *const ent)
{
	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	const size_t idx = group_pos(group, attr, ent);
	if(idx >= group_count(group, attr))
	{
		gmserr = "Not found in group.";
		return 0;
	}

	mowgli_index_delete(attrs, idx, 1);
	return 1;
}


inline
int group_add(group_t *const group,
              const group_attr_t attr,
              void *const ent)
{
	if(group_has(group, attr, ent))
	{
		gmserr = "Already exists in group.";
		return 0;
	}

	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	mowgli_index_append(attrs, ent);
	return 1;
}


inline
int group_has(const group_t *const group,
              const group_attr_t attr,
              const void *const ent)
{
	int equals(const void *const e)
	{
		return e == ent;
	}

	return group_until(group, attr, equals);
}


inline
size_t group_count(const group_t *const group,
                   const group_attr_t attr)
{
	const group_attrs_t *const attrs = group_attrs(group, attr);
	return mowgli_index_count((group_attrs_t *)attrs);
}


inline
void *group_at_mutable(group_t *const group,
                       const group_attr_t attr,
                       const size_t idx)
{
	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	return mowgli_index_get(attrs, idx);
}


inline
const void *group_at(const group_t *const group,
                     const group_attr_t attr,
                     const size_t idx)
{
	const group_attrs_t *const attrs = group_attrs(group, attr);
	return mowgli_index_get((group_attrs_t *)attrs, idx);
}


inline
void *group_find_mutable(group_t *const group,
                         const group_attr_t attr,
                         int (*const func)(const void *))
{
	const size_t idx = group_index(group, attr, func);
	if(idx >= group_count(group, attr))
		return NULL;

	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	return mowgli_index_get(attrs, idx);
}


inline
const void *group_find(const group_t *const group,
                       const group_attr_t attr,
                       int (*const func)(const void *))
{
	const size_t idx = group_index(group, attr, func);
	if(idx >= group_count(group, attr))
		return NULL;

	const group_attrs_t *const attrs = group_attrs(group, attr);
	return mowgli_index_get((group_attrs_t *)attrs, idx);
}


inline
void group_remove_if(group_t *const group,
                     const group_attr_t attr,
                     int (*const func)(void *))
{
	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	for(ssize_t i = mowgli_index_count(attrs) - 1; i > -1; i--)
		if(func(mowgli_index_get(attrs, i)))
			mowgli_index_delete(attrs, i, 1);
}


inline
size_t group_pos(const group_t *const group,
                 const group_attr_t attr,
                 const void *const elem)
{
	int match(const void *const ent)
	{
		return elem == ent;
	}

	return group_index(group, attr, match);
}


inline
size_t group_index(const group_t *const group,
                   const group_attr_t attr,
                   int (*const func)(const void *))
{
	size_t idx = 0;
	int find(const void *const ent)
	{
		if(func(ent))
			return 1;

		idx++;
		return 0;
	}

	group_until(group, attr, find);
	return idx;
}


inline
int group_until_mutable(group_t *const group,
                        const group_attr_t attr,
                        int (*const func)(void *))
{
	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	for(size_t i = 0; i < mowgli_index_count(attrs); i++)
		if(func(mowgli_index_get(attrs, i)))
			return 1;

	return 0;
}


inline
int group_until(const group_t *const group,
                const group_attr_t attr,
                int (*const func)(const void *))
{
	group_attrs_t *const attrs = (group_attrs_t *)group_attrs(group, attr);
	for(size_t i = 0; i < mowgli_index_count(attrs); i++)
		if(func(mowgli_index_get(attrs, i)))
			return 1;

	return 0;
}


inline
void group_foreach_mutable(group_t *const group,
                           const group_attr_t attr,
                           void (*const func)(void *))
{
	group_attrs_t *const attrs = group_attrs_mutable(group, attr);
	for(size_t i = 0; i < mowgli_index_count(attrs); i++)
		func(mowgli_index_get(attrs,i));
}


inline
void group_foreach(const group_t *const group,
                   const group_attr_t attr,
                   void (*const func)(const void *))
{
	group_attrs_t *const attrs = (group_attrs_t *)group_attrs(group, attr);
	for(size_t i = 0; i < mowgli_index_count(attrs); i++)
		func(mowgli_index_get(attrs, i));
}


inline
group_attrs_t *group_attrs_mutable(group_t *const group,
                                   const group_attr_t attr)
{
	return group->attr + attr;
}


inline
const group_attrs_t *group_attrs(const group_t *const group,
                                 const group_attr_t attr)
{
	return group->attr + attr;
}


inline
size_t gms_dbkey_group(const group_attr_t attr,
                       char *const buf)
{
	return sprintf(buf, "%s%zu", GMS_DBKEY_GROUP, attr);
}
