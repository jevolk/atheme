/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef enum
{
	GA_FOUNDER      = 0x0001,
	GA_SUCCESSOR    = 0x0002,
	GA_CONTACT      = 0x0004,
	GA_FLAGS        = 0x0008,
	GA_CHANADMIN    = 0x0010,
	GA_USERADMIN    = 0x0020,
	GA_OPERATOR     = 0x0040,
	GA_INVITE       = 0x0080,
	GA_SET          = 0x0100,
	GA_CLOAK        = 0x0200,
}
access_flag_t;

struct access_flag
{
	access_flag_t flag;
	const char *reflection;
}
const *access_table; // gms.c

static inline
access_flag_t access_char_to_flag(const char letter)
{
	return access_table[letter].flag;
}

static inline
const char *access_char_reflect(const char letter)
{
	return access_table[letter].reflection;
}

mask_vtable_t access_vtable =
{
	NULL,
	access_char_to_flag,
	NULL,
	access_char_reflect,
};

typedef struct
{
	myuser_t *user;
	time_t modified;
	uint mask;
	char role[32];
}
access_t;

uint access_update(access_t *acc, const char *delta);
void access_init(access_t *acc, myuser_t *user, uint mask, const char *role);
access_t *access_new(myuser_t *user, uint mask, const char *role);
void access_delete(void *acc);

int access_read(database_handle_t *db, const char *type, void **elem);
void access_write(database_handle_t *db, const void *elem);


inline
void access_write(database_handle_t *const db,
                  const void *const elem)
{
	const access_t *const acc = elem;

	char mask_buf[32];
	mask_to_str(&access_vtable, acc->mask, mask_buf);

	db_write_word(db, entity(acc->user)->name);
	db_write_time(db, acc->modified);
	db_write_word(db, mask_buf);
	db_write_word(db, acc->role);
}


inline
int access_read(database_handle_t *const db,
                const char *const type,
                void **const elem)
{
	const char *const user_name = db_sread_word(db);
	const time_t modified = db_sread_time(db);
	const char *const mask = db_sread_word(db);
	const char *const role = db_sread_word(db);

	myuser_t *const user = myuser_find(user_name);
	if(!user)
	{
		gmserr = "User not found.";
		return 0;
	}

	*elem = mowgli_alloc(sizeof(access_t));
	access_t *acc = *(access_t **)elem;
	acc->user = user;
	acc->modified = modified;
	acc->mask = mask_from_str(&access_vtable, mask);
	mowgli_strlcpy(acc->role, role, sizeof(acc->role));
	return 1;
}


inline
access_t *access_new(myuser_t *const user,
                     uint mask,
                     const char *const role)
{
	access_t *const ret = mowgli_alloc(sizeof(access_t));
	access_init(ret, user, mask, role);
	return ret;
}


inline
void access_delete(void *const acc)
{
	mowgli_free(acc);
}


inline
void access_init(access_t *const acc,
                 myuser_t *const user,
                 const uint mask,
                 const char *const role)
{
	acc->user = user;
	acc->modified = CURRTIME;
	acc->mask = mask;

	if(role)
		mowgli_strlcpy(acc->role, role, sizeof(acc->role));
	else
		memset(acc->role, 0x0, sizeof(acc->role));
}


inline
uint access_update(access_t *const acc,
                   const char *const delta)
{
	mask_delta(&access_vtable, delta, &acc->mask);
	return acc->mask;
}
