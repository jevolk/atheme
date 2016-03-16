/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef enum
{
	SESSION_TERMINATE  = 0x00,     // Absence of SESSION_CONTINUE means session is over.
	SESSION_CONTINUE   = 0x01,     // Indicates session should continue.
	SESSION_IGNORE     = 0x02,     // Return to the normal command handler.
	SESSION_ERROR      = 0x04,     // Gives user the gmserr string.
}
session_code_t;

typedef enum
{
	SESSION_INPUT_ANY,
	SESSION_INPUT_YES,
	SESSION_INPUT_BOOL,
	SESSION_INPUT_ALPHA,
	SESSION_INPUT_NUMERIC,
	SESSION_INPUT_ALPHANUMERIC,
}
session_input_t;

struct session_t;
typedef void (*session_terminate_t)(struct session_t *);
typedef session_code_t (*session_handler_t)(struct session_t *, sourceinfo_t *, char *);

typedef struct session_t
{
	myuser_t *user;
	time_t started;                    // time session began
	time_t out, in;                    // time last query sent; time of last response
	uint errors;
	session_input_t fmt;
	session_handler_t handler;
	session_terminate_t terminate;
	void *priv;
}
session_t;

session_code_t session_validate(session_t *session, const char *input);
session_code_t session_handle(session_t *session, sourceinfo_t *si, char *input);
void session_query(session_t *session, const session_input_t fmt, session_handler_t handler, const char *query);
void session_init(session_t *session, myuser_t *user);
void session_fini(session_t *session);


inline
void session_fini(session_t *const session)
{
	if(session->terminate)
		session->terminate(session);
}


inline
void session_init(session_t *const s,
                  myuser_t *const user)
{
	s->user = user;
	s->started = CURRTIME;
	s->out = 0;
	s->in = 0;
	s->fmt = SESSION_INPUT_ANY;
	s->errors = 0;
	s->handler = NULL;
	s->terminate = NULL;
	s->priv = NULL;
}


inline
void session_query(session_t *const session,
                   const session_input_t fmt,
                   session_handler_t handler,
                   const char *const query)
{
	session->fmt = fmt;
	session->handler = handler;
	session->out = CURRTIME;
	session->errors = 0;

	myuser_notice(myservice->nick, session->user, "%s%s",
	              query,
	              fmt == SESSION_INPUT_BOOL?   " (y/n)":
	              fmt == SESSION_INPUT_YES?    " (Y/n)":
	                                           "");
}


inline
session_code_t session_handle(session_t *const session,
                              sourceinfo_t *const si,
                              char *const input)
{
	session->in = CURRTIME;

	if(!session->handler)
	{
		gmserr = "I'm sorry but there's been an error and the session has ended.";
		return SESSION_ERROR;
	}

	if(strlen(input) == 1 && input[0] == 0x03)
	{
		gmserr = "Interrupted.";
		return SESSION_ERROR;
	}

	const session_code_t val = session_validate(session, input);
	if(val & SESSION_ERROR)
		session->errors++;

	if(~val & SESSION_CONTINUE || val & SESSION_ERROR)
		return val;

	return session->handler(session, si, input);
}


inline
session_code_t session_validate(session_t *const session,
                                const char *const input)
{
	switch(session->fmt)
	{
		case SESSION_INPUT_BOOL:
		{
			if(irccasecmp(input, "Y") != 0 && irccasecmp(input, "N") != 0)
			{
				gmserr = "Please respond by typing Y or N.";
				return SESSION_CONTINUE | SESSION_ERROR;
			}

			return SESSION_CONTINUE;
		}

		case SESSION_INPUT_YES:
		{
			if(irccasecmp(input, "Y") != 0)
				return SESSION_IGNORE;

			return SESSION_CONTINUE;
		}

		case SESSION_INPUT_ANY:
		default:
			return SESSION_CONTINUE;
	}
}
