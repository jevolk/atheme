/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */


const char *reflect_direction(const mowgli_eventloop_io_dir_t s)
{
	switch(s)
	{
		case MOWGLI_EVENTLOOP_IO_READ:       return "READ";
		case MOWGLI_EVENTLOOP_IO_WRITE:      return "WRITE";
		case MOWGLI_EVENTLOOP_IO_ERROR:      return "ERROR";
		default:                             return "?????";
	}
}


const char *reflect_status(const ConnStatusType s)
{
	switch(s)
	{
		case CONNECTION_OK:                  return "OK";
		case CONNECTION_BAD:                 return "BAD";
		case CONNECTION_STARTED:             return "STARTED";
		case CONNECTION_MADE:                return "MADE";
		case CONNECTION_AWAITING_RESPONSE:   return "AWAITING_RESPONSE";
		case CONNECTION_AUTH_OK:             return "AUTH_OK";
		case CONNECTION_SETENV:              return "SETENV";
		case CONNECTION_SSL_STARTUP:         return "SSL_STARTUP";
		case CONNECTION_NEEDED:              return "NEEDED";
		default:                             return "??????";
	}
}


const char *reflect_polling(const PostgresPollingStatusType s)
{
	switch(s)
	{
		case PGRES_POLLING_OK:               return "OK";
		case PGRES_POLLING_FAILED:           return "FAILED";
		case PGRES_POLLING_READING:          return "READING";              /* These two indicate that one may    */
		case PGRES_POLLING_WRITING:          return "WRITING";              /* use select before polling again.   */
		case PGRES_POLLING_ACTIVE:           return "ACTIVE";               /* unused; keep for awhile for backwards compatibility */
		default:                             return "???????";
	}
}


const char *reflect_exec(const ExecStatusType s)
{
	switch(s)
	{
		case PGRES_EMPTY_QUERY:              return "EMPTY_QUERY";          /* empty query string was executed */
		case PGRES_COMMAND_OK:               return "COMMAND_OK";           /* a query command that doesn't                                                                                     return  anything was executed properly by the backend */
		case PGRES_TUPLES_OK:                return "TUPLES_OK";            /* a query command that returns tuples was executed properly by the backend, PGresult contains the result tuples */
		case PGRES_COPY_OUT:                 return "COPY_OUT";             /* Copy Out data transfer in progress */
		case PGRES_COPY_IN:                  return "COPY_IN";              /* Copy In data transfer in progress */
		case PGRES_BAD_RESPONSE:             return "BAD_RESPONSE";         /* an unexpected response was recv'd from the backend */
		case PGRES_NONFATAL_ERROR:           return "NONFATAL_ERROR";       /* notice or warning message */
		case PGRES_FATAL_ERROR:              return "FATAL_ERROR";          /* query failed */
		case PGRES_COPY_BOTH:                return "COPY_BOTH";            /* Copy In/Out data transfer in progress */
		case PGRES_SINGLE_TUPLE:             return "SINGLE_TUPLE";         /* single tuple from larger resultset */
		default:                             return "???????";
	}
}


const char *reflect_trans(const PGTransactionStatusType s)
{
	switch(s)
	{
		case PQTRANS_IDLE:                   return "IDLE";                 /* connection idle */
		case PQTRANS_ACTIVE:                 return "ACTIVE";               /* command in progress */
		case PQTRANS_INTRANS:                return "INTRANS";              /* idle, within transaction block */
		case PQTRANS_INERROR:                return "INERROR";              /* idle, within failed transaction */
		case PQTRANS_UNKNOWN:                return "UNKNOWN";              /* cannot determine status */
		default:                             return "???????";              // neither can we
	}
}


const char *reflect_verbosity(const PGVerbosity s)
{
	switch(s)
	{
		case PQERRORS_TERSE:                 return "TERSE";                /* single-line error messages */
		case PQERRORS_DEFAULT:               return "DEFAULT";              /* recommended style */
		case PQERRORS_VERBOSE:               return "VERBOSE";              /* all the facts, ma'am */
		default:                             return "???????";
	}
}


const char *reflect_ping(const PGPing s)
{
	switch(s)
	{
		case PQPING_OK:                      return "OK";                   /* server is accepting connections */
		case PQPING_REJECT:                  return "REJECT";               /* server is alive but rejecting connections */
		case PQPING_NO_RESPONSE:             return "NO_RESPONSE";          /* could not establish connection */
		case PQPING_NO_ATTEMPT:              return "NO_ATTEMPT";           /* connection not attempted (bad params) */
		default:                             return "???????";
	}
}


const char *reflect_event(const PGEventId s)
{
	switch(s)
	{
		case PGEVT_REGISTER:                 return "REGISTER";
		case PGEVT_CONNRESET:                return "CONNRESET";
		case PGEVT_CONNDESTROY:              return "CONNDESTROY";
		case PGEVT_RESULTCREATE:             return "RESULTCREATE";
		case PGEVT_RESULTCOPY:               return "RESULTCOPY";
		case PGEVT_RESULTDESTROY:            return "RESULTDESTROY";
		default:                             return "????????";
	}
}
