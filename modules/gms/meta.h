/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef struct
{
	object_t md;
	const char *dbkey;
}
meta_t;

const char *meta_get(const meta_t *meta, const char *key);
void meta_set(meta_t *meta, const char *key, const char *val);
int meta_del(meta_t *meta, const char *key);

void meta_foreach(const meta_t *meta, void (*func)(const char *key, const char *val));
int meta_until(const meta_t *meta, int (*func)(const char *key, const char *val));

void meta_read(meta_t *meta, database_handle_t *db, const char *type);
void meta_write(meta_t *meta, database_handle_t *const db);

void meta_dtor(void *const o);
void meta_init(meta_t *meta, const char *dbkey);
void meta_fini(meta_t *meta);


inline
void meta_fini(meta_t *const meta)
{
	object_dispose(&meta->md);
}


inline
void meta_init(meta_t *const meta,
               const char *const dbkey)
{
	meta->dbkey = dbkey;
	object_init(&meta->md, dbkey, meta_dtor);
}


inline
void meta_write(meta_t *const meta,
                database_handle_t *const db)
{
	const void *md;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(md, &state, ((meta_t *)meta)->md.metadata)
	{
		db_start_row(db, meta->dbkey);
		db_write_word(db, ((const metadata_t *)md)->name);
		db_write_str(db, ((const metadata_t *)md)->value);
		db_commit_row(db);
	}
}


inline
void meta_read(meta_t *const meta,
               database_handle_t *const db,
               const char *const type)
{
	const char *const key = db_sread_word(db);
	const char *const val = db_sread_str(db);
	metadata_add(&meta->md, key, val);
}


inline
void meta_dtor(void *const o)
{
	if(object(o)->metadata)
		metadata_delete_all(object(o));
}


inline
int meta_until(const meta_t *const meta,
               int (*const func)(const char *key, const char *val))
{
	return metadata_until(object(meta), func);
}


inline
void meta_foreach(const meta_t *const meta,
                  void (*const func)(const char *key, const char *val))
{
	metadata_foreach(object(meta), func);
}


inline
int meta_del(meta_t *const meta,
             const char *const key)
{
	metadata_delete(object(meta), key);
	return 1;
}


inline
void meta_set(meta_t *const meta,
              const char *const key,
              const char *const val)
{
	metadata_add(object(meta), key, val);
}


inline
const char *meta_get(const meta_t *const meta,
                     const char *const key)
{
	const metadata_t *const md = metadata_find(object(meta), key);
	return md? md->value : NULL;
}
