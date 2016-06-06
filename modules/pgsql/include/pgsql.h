/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations, idgaf:
 * - Atheme Development Group
 *
 *
 * You may directly link to the singleton instance of struct PG which multiplexes
 * all database connections and allows central database administration.
 *
 * !!! YOU DO NOT NEED TO CREATE YOUR OWN INSTANCE OF struct PG.
 *
 * Use the interface mostly in pg.h to find or create and query your database
 * session. Filling in the callbacks directly in the conn_t structure (conn.h)
 * and using the observers defined there is also an acceptable usage.
 * 
 * !!! NOTE:
 *
 * 1. You may need to declare `service_t *myservice` upstream from this include,
 *    setting it to the valid value for your module.
 *
 */


#include <postgresql/libpq-fe.h>
#include <postgresql/libpq-events.h>


#ifdef PG_MODULE_INTERNAL

	#include <atheme.h>
	#include <uplink.h>

	service_t *myservice;
	#include "colors.h"
	#include "util.h"

#endif // PG_MODULE_INTERNAL


const char *pgerr;
#include "reflect.h"
#include "conn.h"
#include "conns.h"
#include "sess.h"
#include "sesss.h"
#include "pg.h"


#ifdef PG_MODULE_INTERNAL

PG *pg;

static
void module_init_common(module_t *const m)
{
	void *sym_ptr;
	MODULE_TRY_REQUEST_SYMBOL(m, sym_ptr, "pgsql/pgsql", "pg");
	pg = *((PG **)sym_ptr);
	myservice = pg->svc;
	pgerr = "";
}

static
void module_fini_common(module_unload_intent_t intent)
{
}

#endif // PG_MODULE_INTERNAL
