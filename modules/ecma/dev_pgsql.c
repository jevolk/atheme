/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"
#include "pgsql.h"


PG *pg;


// Util.
v8::Local<v8::Object> get_priv(conn_t &);            // fd descriptor is stored in conn->priv
v8::Local<v8::Object> get_priv(conn_t *const &);
conn_t *connect(myuser_t *const &owner, const v8::Local<v8::Object> &opts);

// PG callbacks
void handle_status(conn_t *, ConnStatusType) noexcept;
void handle_result(conn_t *, const PGresult *) noexcept;
void handle_notice(conn_t *, const PGresult *, const char *msg) noexcept;
void handle_notify(conn_t *, const char *name, const int pid, const char *val) noexcept;
int handle_event(conn_t *, const PGEventId, void *data) noexcept;

// dev.pgsql filesystem presence
struct pgsql
:welt
{
	struct connection
	:structure<conn_t>
	{
		call_ret call(conn_t &, const call_arg &) override;
		void dtor(conn_t &, const call_arg &) override;
		conn_t *init(const call_arg &) override;
		connection();
	}
	connection;

	get_ret iget(const get_arg &arg, const uint32_t &) override;
	del_ret idel(const del_arg &arg, const uint32_t &) override;
	qry_ret iqry(const qry_arg &arg, const uint32_t &) override;
	enu_ret ienu(const enu_arg &arg) override;
	call_ret ctor(const call_arg &arg) override;
}
static pgsql;


DECLARE_MODULE_V1
(
	"ecma/dev_pgsql",
	MODULE_UNLOAD_CAPABILITY_OK,
	[](module_t *const m) noexcept
	{
		module_register(m);
		pg = *link_symbol<PG>("pgsql/pgsql", "pg");
		ecma->add("dev.pgsql", &pgsql);
	},
	[](module_unload_intent_t) noexcept
	{
		ecma->del("dev.pgsql");
	},
	PACKAGE_STRING,
	"jzk"
);


pgsql::call_ret
pgsql::ctor(const call_arg &arg)
{
	return connection(current(arg), function::make_argv(arg));
}


pgsql::enu_ret
pgsql::ienu(const enu_arg &arg)
{
	size_t i(0);
	mowgli_node_t *n, *tn;
	LS1::array ret(conns_count(&pg->conns));
	MOWGLI_LIST_FOREACH_SAFE(n, tn, pg->conns.list.head)
	{
		const auto conn(reinterpret_cast<conn_t *>(n->data));
		::set(ret, i++, LS1::integer(conn_id(conn)));
	}

	return ret;
}


pgsql::qry_ret
pgsql::iqry(const qry_arg &arg,
            const uint32_t &idx)
{
	if(pg_conn_find(pg, idx))
		return LS1::integer(v8::None);

	return {};
}


pgsql::del_ret
pgsql::idel(const del_arg &arg,
            const uint32_t &idx)
{
	auto &dasein(dasein::get(arg));
	const auto conn(pg_conn_find_mutable(pg, idx));
	if(!conn)
		throw error<ENOENT>();

	if(dasein.get_owner() != conn->owner)
		throw error<EACCES>();

	rede::close(get_priv(conn));
	return LS1::boolean(true);
}


pgsql::get_ret
pgsql::iget(const get_arg &arg,
            const uint32_t &idx)
{
	const auto conn(pg_conn_find_mutable(pg, idx));
	if(!conn)
		throw error<ENOENT>("No connection found with ID %u.", idx);

	return get_priv(conn);
}


pgsql::connection::connection()
:structure<conn_t>
{{

}}
{
}

/*
	switch(hash(string(name)))
	{
		case hash("id"):
			return LS1::integer(conn_id(&c));

		case hash("creation"):
			return LS1::integer(c.creation);

		case hash("owner"):
			if(!c.owner)
				return {};

			return LS1::string(entity(c.owner)->name);

		case hash("tty"):
			if(!c.tty)
				return {};

			return LS1::string(c.tty->nick);

		case hash("busy"):
			return LS1::boolean(conn_busy(&c));

		case hash("status"):
			return LS1::string(reflect_status(conn_status(&c)));

		case hash("trans_status"):
			return LS1::string(reflect_trans(conn_trans_status(&c)));

		case hash("errmsg"):
			return LS1::string(conn_errmsg(&c)?: "No error message");

		case hash("queries_sent"):
			return LS1::integer(c.queries_sent);

		case hash("results_recv"):
			return LS1::integer(c.results_recv);

		case hash("notifies_recv"):
			return LS1::integer(c.notifies_recv);

		default:
			return {};
	}
*/


conn_t *
pgsql::connection::init(const call_arg &arg)
{
	using namespace v8;

	auto &dasein(dasein::get(current(arg)));
	auto owner(dasein.get_owner());

	auto opts(mustbe<Object>(arg[0], "Options map required: {'key': 'val', ... }"));
	auto conn(connect(owner, opts));
	conn->handle_status = handle_status;
	conn->handle_result = handle_result;
	conn->handle_notice = handle_notice;
	conn->handle_notify = handle_notify;
	conn->handle_event_any = handle_event;

	// Open a descriptor and wrap this object.
	// When the descriptor destructs it will call the dtor() here.
	auto fd(rede::open(instance(arg)));

	// Save the descriptor in the priv ptr. When PGSQL enters the
	// callbacks we read this to get our bearings. This must be
	// manually deleted in the dtor.
	conn->priv = new v8::Global<v8::Object>(isolate(), fd);

	return conn;
}


void
pgsql::connection::dtor(conn_t &conn,
                        const call_arg &arg)
{
	auto *const descriptor(static_cast<v8::Global<v8::Object> *>(conn.priv));
	const std::unique_ptr<v8::Global<v8::Object>> scope(descriptor);
	pg_conn_delete(pg, &conn);
}


pgsql::connection::call_ret
pgsql::connection::call(conn_t &conn,
                        const call_arg &arg)
{
	auto query_string(as<v8::String>(arg[0]));
	conn_query(&conn, string(query_string));
	return instance(arg);
}



struct entry_scope
{
	struct isolate_scope isolate_scope;
	struct handle_scope handle_scope;
	v8::Local<v8::Object> fd;
	struct context_scope context_scope;

	entry_scope(conn_t *const &c);
};

entry_scope::entry_scope(conn_t *const &c)
:fd{get_priv(c)}
,context_scope{creator(fd)}
{
}



void handle_status(conn_t *const c,
                   ConnStatusType status)
noexcept
{
	printf("PG status (%s) (%s)\n", reflect_status(status), conn_errmsg(c));

	entry_scope entry_scope(c);
	auto &fd(entry_scope.fd);
	switch(status)
	{
		case CONNECTION_OK:
		case CONNECTION_BAD:
		default:
			rede::result(fd, rede::described(fd));
			return;
	}
}


int handle_event(conn_t *const c,
                 const PGEventId event,
                 void *const data)
noexcept
{
	printf("PG result\n");

	entry_scope entry_scope(c);
	auto &fd(entry_scope.fd);

	return true;
}


void handle_notify(conn_t *const c,
                   const char *const name,
                   const int pid,
                   const char *const val)
noexcept
{
	printf("PG notify\n");

	entry_scope entry_scope(c);
}


void handle_notice(conn_t *const c,
                   const PGresult *const result,
                   const char *const msg)
noexcept
{
	printf("PG notice (%p) (%s)\n", result, msg);

	entry_scope entry_scope(c);
}


void handle_result(conn_t *const c,
                   const PGresult *const result)
noexcept
{
	using namespace v8;

	printf("PG result\n");

	entry_scope entry_scope(c);
	auto &fd(entry_scope.fd);

	const auto status(PQresultStatus(result));
	const auto nt(PQntuples(result));
	const auto nf(PQnfields(result));
	LS1::array res(nt);
	for(ssize_t i(0); i < nt; i++)
	{
		LS1::array row(nf);
		for(ssize_t j(0); j < nf; j++)
		{
			const char *const value(PQgetvalue(result, i, j));
			set(row, j, LS1::string(value));
		}

		set(res, i, row);
	}

	rede::result(fd, res);
}


conn_t *connect(myuser_t *const &owner,
                const v8::Local<v8::Object> &opts)
{
	using namespace v8;

	if(!owner)
		throw error<EACCES>("Process must have a valid owner for this operation");

	size_t i(0);
	static const size_t KEYMAX(16);
	std::array<const char *, KEYMAX> keys {0};
	std::array<const char *, KEYMAX> vals {0};
	for_each<Value>(opts, [&i, &keys, &vals]
	(Local<Name> &key, Local<Value> val)
	{
		if(i >= KEYMAX - 1)
			throw error<EINVAL>("too many options");

		static const size_t KEYSIZE(64);
		static thread_local char keybuf[KEYMAX][KEYSIZE];
		static thread_local char valbuf[KEYMAX][BUFSIZE];
		mowgli_strlcpy(keybuf[i], string(key), sizeof(keybuf[i]));
		mowgli_strlcpy(valbuf[i], string(val), sizeof(valbuf[i]));
		keys[i] = keybuf[i];
		vals[i] = valbuf[i];
		++i;
	});

	auto conn(pg_conn_new(pg, owner, keys.data(), vals.data()));
	if(!conn && pgerr)
		throw error<EINVAL>(pgerr);

	if(!conn)
		throw error<EHOSTUNREACH>();

	return conn;
}


inline
v8::Local<v8::Object>
get_priv(conn_t *const &c)
{
	return get_priv(*c);
}


inline
v8::Local<v8::Object>
get_priv(conn_t &c)
{
	auto *const global(reinterpret_cast<v8::Global<v8::Object> *>(c.priv));
	return local(*global);
}
