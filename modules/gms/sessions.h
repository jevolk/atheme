/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef struct
{
	mowgli_dictionary_t *sessions;   // myuser_t * => session_t *
}
sessions_t;

ptrdiff_t sessions_cmp(const void *a, const void *b);
session_t *sessions_find(sessions_t *sessions, const myuser_t *user);
int sessions_del(sessions_t *sessions, const session_t *session);
int sessions_add(sessions_t *sessions, session_t *session);
int sessions_end(sessions_t *sessions, session_t *session);

int sessions_end_session(sessions_t *sessions, myuser_t *user);
session_t *sessions_new_session(sessions_t *sessions, myuser_t *user);
int sessions_handle(sessions_t *session, sourceinfo_t *si, char *input);

session_t *sessions_query(sessions_t *sessions, myuser_t *user, session_handler_t handler, const char *query);

void sessions_init(sessions_t *sessions);
void sessions_fini(sessions_t *sessions);


inline
session_t *sessions_query(sessions_t *const sessions,
                          myuser_t *const user,
                          session_handler_t handler,
                          const char *const query)
{
	session_t *session = sessions_find(sessions, user);
	if(!session)
		session = sessions_new_session(sessions, user);

	session_query(session, SESSION_INPUT_ANY, handler, query);
	return session;
}


inline
void sessions_fini(sessions_t *const sessions)
{
	void dtor(mowgli_dictionary_elem_t *const elem, void *const priv)
	{
		session_t *const session = elem->data;
		session_fini(session);
		mowgli_free(session);
	}

	mowgli_dictionary_destroy(sessions->sessions, dtor, NULL);
}


inline
void sessions_init(sessions_t *const sessions)
{
	sessions->sessions = mowgli_dictionary_create(sessions_cmp);
}


inline
int sessions_handle(sessions_t *const sessions,
                    sourceinfo_t *const si,
                    char *const input)
{
	if(!si->smu)
		return 0;

	session_t *const session = sessions_find(sessions, si->smu);
	if(!session)
		return 0;

	session_code_t code = session_handle(session, si, input);
	if((code & SESSION_ERROR) && gmserr)
		command_success_nodata(si, "%c%c%02derror:%c %s", COLOR_BOLD, COLOR_ON, FG_RED, COLOR_OFF, gmserr);

	if(session->errors > 3)
	{
		command_success_nodata(si, "This session has encountered too many errors.");
		code &= ~SESSION_CONTINUE;
	}

	if(~code & SESSION_CONTINUE)
	{
		command_success_nodata(si, "\2***\2 This query session has ended. You may continue using normal commands again. \2***\2");
		sessions_end(sessions, session);
	}

	return code & SESSION_IGNORE? 0 : 1;
}


inline
session_t *sessions_new_session(sessions_t *const sessions,
                                myuser_t *const user)
{
	session_t *const session = mowgli_alloc(sizeof(session_t));
	session_init(session, user);
	return sessions_add(sessions, session)? session : NULL;
}


inline
int sessions_end_session(sessions_t *const sessions,
                         myuser_t *const user)
{
	session_t *const session = sessions_find(sessions, user);
	if(!session)
	{
		gmserr = "No session found for user.";
		return 0;
	}

	return sessions_end(sessions, session);
}


inline
int sessions_end(sessions_t *const sessions,
                 session_t *const session)
{
	sessions_del(sessions, session);
	session_fini(session);
	mowgli_free(session);
	return 1;
}


inline
int sessions_add(sessions_t *const sessions,
                 session_t *const session)
{
	if(!mowgli_dictionary_add(sessions->sessions, session->user, session))
	{
		gmserr = "Failed to add new session.";
		return 0;
	}

	return 1;
}


inline
int sessions_del(sessions_t *const sessions,
                 const session_t *const session)
{
	return mowgli_dictionary_delete(sessions->sessions, session->user) != NULL;
}


inline
session_t *sessions_find(sessions_t *const sessions,
                         const myuser_t *const user)
{
	mowgli_dictionary_elem_t *const elem = mowgli_dictionary_find(sessions->sessions, user);
	return elem? elem->data : NULL;
}


inline
ptrdiff_t sessions_cmp(const void *const a,
                       const void *const b)
{
	return a - b;
}
