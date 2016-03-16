/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

struct interactive_t;
typedef void (*interactive_term_t)(struct interactive_t *);
typedef int (*interactive_condition_t)(const struct interactive_t *);
typedef void (*interactive_query_gen_t)(struct interactive_t *, char *, size_t);
typedef session_code_t (*interactive_handler_t)(struct interactive_t *, sourceinfo_t *, char *);

typedef struct form_t
{
	const char *query;                          // Simple fixed string query.
	interactive_handler_t handler;              // Response handler.
	session_input_t fmt;                        // Pre-checks the response for conformity before handler called.
	const struct form_t *next;                  // The next form to goto.
	interactive_condition_t condition;          // If true, this query form will be skipped for *next.
	interactive_query_gen_t query_generator;    // This may be called to generate complex query strings.
}
form_t;

typedef struct interactive_t
{
	session_t *session;                         // Session instance
	uint handled;                               // Counter of handled queries

	const form_t *form;
	interactive_term_t terminate;
	void *priv;
}
interactive_t;

void interactive_terminator(session_t *session);
void interactive_advance(interactive_t *inter);
uint interactive_remaining(const interactive_t *inter);
size_t interactive_make_query(interactive_t *inter, char *buf, const size_t max);
session_code_t interactive_handler(session_t *session, sourceinfo_t *si, char *msg);

void interactive_end(sessions_t *sessions, interactive_t *inter);
interactive_t *interactive_start(sessions_t *sessions, const form_t *form, myuser_t *user, void *const priv);


inline
interactive_t *interactive_start(sessions_t *const sessions,
                                 const form_t *const form,
                                 myuser_t *const user,
                                 void *const priv)
{
	interactive_t *const inter = mowgli_alloc(sizeof(interactive_t));
	inter->handled = 0;
	inter->form = form;
	inter->terminate = NULL;
	inter->priv = priv;

	if(inter->form->condition && inter->form->condition(inter))
		interactive_advance(inter);

	if(!inter->form)
	{
		// form was fully satisfied non-interactively
		mowgli_free(inter);
		return NULL;
	}

	inter->session = sessions_new_session(sessions, user);
	inter->session->terminate = interactive_terminator;
	inter->session->priv = inter;

	char query[512];
	interactive_make_query(inter, query, sizeof(query));
	session_query(inter->session, inter->form->fmt, interactive_handler, query);

	return inter;
}


inline
void interactive_end(sessions_t *const sessions,
                     interactive_t *const inter)
{
	if(inter->session)
	{
		sessions_end(sessions, inter->session);
		inter->session = NULL;
	}
}


inline
session_code_t interactive_handler(session_t *const session,
                                   sourceinfo_t *const si,
                                   char *const msg)
{
	interactive_t *const inter = session->priv;
	const form_t *const form = inter->form;
	if(!form)
		return SESSION_TERMINATE;

	if(form->handler)
	{
		inter->handled++;
		const session_code_t code = form->handler(inter, si, msg);
		if(~code & SESSION_CONTINUE)
			return code;
	}

	// If the user did not change the form in the handler we can traverse normally.
	if(inter->form == form)
		interactive_advance(inter);

	if(!inter->form)
		return SESSION_TERMINATE;

	static char query[512];
	if(!interactive_make_query(inter, query, sizeof(query)))
	{
		gmserr = "Failed to make query.";
		return SESSION_TERMINATE;
	}

	session_query(session, inter->form->fmt, interactive_handler, query);
	return SESSION_CONTINUE;
}


inline
size_t interactive_make_query(interactive_t *const inter,
                              char *const buf,
                              const size_t max)
{
	static char gbuf[512];
	if(!inter->form->query)
	{
		if(inter->form->query_generator)
			inter->form->query_generator(inter, gbuf, sizeof(gbuf));
		else
			return 0;
	}

	return snprintf(buf, max, "[%u/%u] %c%c%s%c",
	                inter->handled + 1,
	                inter->handled + interactive_remaining(inter),
	                COLOR_ON, COLOR_BOLD,
	                inter->form->query?: gbuf,
	                COLOR_OFF);
}


inline
uint interactive_remaining(const interactive_t *const inter)
{
	uint ret = 0;
	for(const form_t *form = inter->form; form; form = form->next)
		ret++;

	return ret;
}


inline
void interactive_advance(interactive_t *const inter)
{
	do
	{
		inter->form = inter->form->next;
	}
	while(inter->form && inter->form->condition && inter->form->condition(inter));
}


inline
void interactive_terminator(session_t *const session)
{
	interactive_t *const inter = session->priv;

	if(!inter)
		return;

	if(inter->terminate)
		inter->terminate(inter);

	inter->session = NULL;
	mowgli_free(inter);
}
