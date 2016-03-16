/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#define GMS_DBKEY_REQUEST       GMS_NAME "R"
#define GMS_REQUEST_TYPES_MAX   32
#define GMS_REQUEST_SIGS        32

typedef enum
{
	REQUEST_REGISTER,
	REQUEST_JOIN,
	REQUEST_CLOAK,
}
request_type_t;

typedef struct
{
	const char *type;
	size_t size;

	int (*ctor)(void *, void *);
	void (*dtor)(void *);

	void (*write)(const void *, database_handle_t *);
	int (*read)(void *, database_handle_t *);

	int (*cmp)(const void *, const void *);
	void (*signal[GMS_REQUEST_SIGS])(void *, int, char *);
}
request_vtable_t;

typedef struct
{
	const request_vtable_t *this;
	uint id;
	time_t modified;
	myuser_t *user;
	mowgli_eventloop_timer_t *alarm;
}
request_t;

request_vtable_t *request_vtable;

#define request(x) ((request_t *)(x))
uint request_type(const request_t *request);
static const char *request_type_str(const request_t *request);
size_t request_string(const request_t *request, char *buf, const size_t max);

void request_alarm_handler(void *ptr);
void request_set_alarm_fromnow(request_t *request, const time_t reltime);
void request_set_alarm(request_t *request, const time_t abstime);
int request_signal(request_t *request, const int sig, char *data);

int request_init(request_t *request, const uint type, myuser_t *user, void *data);
void request_fini(request_t *request);
request_t *request_new(const request_type_t type, myuser_t *user, void *data);
void request_delete(void *request);

int request_read(database_handle_t *db, const char *type, void **elem);
void request_write(database_handle_t *db, const void *elem);


inline
void request_write(database_handle_t *const db,
                   const void *const elem)
{
	const request_t *const r = elem;
	db_write_uint(db, request_type(r));
	db_write_uint(db, r->id);
	db_write_time(db, r->modified);
	db_write_word(db, entity(r->user)->name);

	if(r->this->write)
		r->this->write(r, db);
}


inline
int request_read(database_handle_t *const db,
                 const char *const type,
                 void **const elem)
{
	const uint typ = db_sread_uint(db);
	const uint id = db_sread_uint(db);
	const time_t modified = db_sread_time(db);
	const char *const user_name = db_sread_word(db);

	myuser_t *const user = myuser_find(user_name);
	if(!user)
	{
		gmserr = "User not found.";
		return 0;
	}

	if(!request_vtable[typ].size)
	{
		gmserr = "Failed to read request type";
		return 0;
	}

	*elem = mowgli_alloc(request_vtable[typ].size);
	request_t *const r = *elem;
	r->this = request_vtable + typ;
	r->id = id;
	r->modified = modified;
	r->user = user;
	r->alarm = NULL;

	if(r->this->read && !r->this->read(r, db))
	{
		gmserr_prepend("Request read failed in child: ");
		mowgli_free(r);
		return 0;
	}

	return 1;
}


inline
int request_signal(request_t *const request,
                   const int signum,
                   char *const data)
{
	if(signum >= GMS_REQUEST_SIGS)
		return 0;

	if(!request->this->signal[signum])
		return 0;

	request->this->signal[signum](request, signum, data);
	return 1;
}


inline
void request_delete(void *const r)
{
	request_t *const request = r;
	request_fini(request);
	mowgli_free(request);
}


inline
request_t *request_new(const request_type_t type,
                       myuser_t *const user,
                       void *const data)
{
	request_t *const request = mowgli_alloc(request_vtable[type].size);
	if(!request_init(request, type, user, data))
	{
		mowgli_free(request);
		return NULL;
	}

	return request;
}


inline
void request_fini(request_t *const request)
{
	if(request->alarm)
		mowgli_timer_destroy(base_eventloop, request->alarm);

	if(request->this->dtor)
		request->this->dtor(request);
}


inline
int request_init(request_t *const request,
                 const uint type,
                 myuser_t *const user,
                 void *const data)
{
	request->this = request_vtable + type;
	request->id = 0;
	request->modified = CURRTIME;
	request->user = user;
	request->alarm = NULL;

	return request->this->ctor? request->this->ctor(request, data) : 1;
}


inline
void request_set_alarm(request_t *const request,
                       const time_t abstime)
{
	const time_t reltime = abstime - CURRTIME;
	request_set_alarm_fromnow(request, reltime);
}


inline
void request_set_alarm_fromnow(request_t *const request,
                               const time_t reltime)
{
	if(request->alarm)
		mowgli_timer_destroy(base_eventloop, request->alarm);

	request->alarm = mowgli_timer_add_once(base_eventloop, NULL, request_alarm_handler, request, reltime);
}


inline
void request_alarm_handler(void *const ptr)
{
	request_t *const request = ptr;
	request->alarm = NULL;
	request_signal(request, SIGALRM, NULL);
}


inline
size_t request_string(const request_t *const r,
                      char *const buf,
                      const size_t max)
{
	return snprintf(buf, max, "#%-3u %-16s %-15s %s ago",
	                          r->id,
	                          request_type_str(r),
	                          r->user? entity(r->user)->name : "<no user>",
	                          time_ago(r->modified));
}


static inline
const char *request_type_str(const request_t *const request)
{
	return !request->this?        "<type error>":
	       !request->this->type?  "<no type description>":
	                              request->this->type;
}


inline
uint request_type(const request_t *const request)
{
	return request->this - request_vtable;
}
