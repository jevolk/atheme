/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"

using argv = function::argv;


static
void p_config_purge(void *const &data, argv &ret)
{
}


static
void p_config_ready(void *const &data, argv &ret)
{
}


static
void p_db_write(void *const &data, argv &ret)
{
}


static
void p_db_write_pre_ca(void *const &data, argv &ret)
{
}


static
void p_db_saved(void *const &data, argv &ret)
{
}


static
void p_db_read(void *const &data, argv &ret)
{
}


static
void p_shutdown(void *const &data, argv &ret)
{
}


static
void p_channel_add(void *const &data, argv &ret)
{
	const auto c(reinterpret_cast<channel_t *>(data));
	ret.emplace_back(LS1::string(c->name));
}


static
void p_channel_delete(void *const &data, argv &ret)
{
	const auto c(reinterpret_cast<channel_t *>(data));
	ret.emplace_back(LS1::string(c->name));
}


static
void p_channel_tschange(void *const &data, argv &ret)
{
	const auto c(reinterpret_cast<channel_t *>(data));
	ret.emplace_back(LS1::string(c->name));
}


static
void p_channel_join(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_channel_joinpart_t *>(data));
	if(!d->cu)
		return;

	ret.emplace_back(LS1::string(d->cu->chan->name));
	ret.emplace_back(LS1::string(d->cu->user->nick));
	ret.emplace_back(LS1::integer(d->cu->modes));
}


static
void p_channel_part(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_channel_joinpart_t *>(data));
	if(!d->cu)
		return;

	ret.emplace_back(LS1::string(d->cu->chan->name));
	ret.emplace_back(LS1::string(d->cu->user->nick));
	ret.emplace_back(LS1::integer(d->cu->modes));
}


static
void p_channel_mode(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<hook_channel_mode_t *>(data));
	ret.emplace_back(LS1::string(d->u? d->u->nick : ""));
	ret.emplace_back(LS1::string(channel_modes(d->c, true)));
}


static
void p_channel_topic(void *const &data, argv &ret)
{
	const auto c(reinterpret_cast<channel_t *>(data));
	ret.emplace_back(LS1::string(c->name));
}


static
void p_channel_can_change_topic(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_channel_topic_check_t *>(data));

}


static
void p_channel_message(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_cmessage_data_t *>(data));
	ret.emplace_back(LS1::string(d->u->nick));
	ret.emplace_back(LS1::string(d->c->name));
	ret.emplace_back(LS1::string(d->msg));
}


static
void p_server_add(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<server_t *>(data));

}


static
void p_server_eob(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<server_t *>(data));

}


static
void p_server_delete(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_server_delete_t *>(data));

}


static
void p_user_add(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_user_nick_t *>(data));
	ret.emplace_back(LS1::string(d->u->nick));
}


static
void p_user_delete(void *const &data, argv &ret)
{
	const auto &u(reinterpret_cast<const user_t *>(data));
	ret.emplace_back(LS1::string(u->nick));
}


static
void p_user_delete_info(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_user_delete_t *>(data));
	ret.emplace_back(LS1::string(d->u->nick));
	ret.emplace_back(LS1::string(d->comment));
}


static
void p_user_nickchange(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_user_nick_t *>(data));
	ret.emplace_back(LS1::string(d->u->nick));
	ret.emplace_back(LS1::string(d->oldnick));
}


static
void p_user_away(void *const &data, argv &ret)
{
	const auto &u(reinterpret_cast<const user_t *>(data));
	ret.emplace_back(LS1::string(u->nick));
}


static
void p_user_deoper(void *const &data, argv &ret)
{
	const auto &u(reinterpret_cast<const user_t *>(data));
	ret.emplace_back(LS1::string(u->nick));
}


static
void p_user_oper(void *const &data, argv &ret)
{
	const auto &u(reinterpret_cast<const user_t *>(data));
	ret.emplace_back(LS1::string(u->nick));
}


static
void p_channel_can_register(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<hook_channel_register_check_t *>(data));
	ret.emplace_back(LS1::string(d->si->smu? entity(d->si->smu)->name : d->si->su->nick));
	ret.emplace_back(LS1::string(d->name));
}


static
void p_channel_drop(void *const &data, argv &ret)
{
	const auto &mc(reinterpret_cast<const mychan_t *>(data));
	ret.emplace_back(LS1::string(mc->name));
}


static
void p_channel_info(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_channel_req_t *>(data));

}


static
void p_channel_register(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_channel_req_t *>(data));

}


static
void p_channel_check_expire(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_expiry_req_t *>(data));

}


static
void p_channel_acl_change(void *const &data, argv &ret)
{
	const auto &d(reinterpret_cast<const hook_channel_acl_req_t *>(data));

}


static
void p_group_drop(void *const &data, argv &ret)
{
	//const auto d(reinterpret_cast<mygroup_t *>(data));

}


static
void p_group_register(void *const &data, argv &ret)
{
	//const auto d(reinterpret_cast<mygroup_t *>(data));

}


static
void p_nick_can_register(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_register_check_t *>(data));

}


static
void p_nick_group(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_req_t *>(data));

}


static
void p_nick_check(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<user_t *>(data));

}


static
void p_nick_enforce(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_nick_enforce_t *>(data));

}


static
void p_nick_ungroup(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_req_t *>(data));

}


static
void p_nick_check_expire(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_expiry_req_t *>(data));

}


static
void p_sasl_input(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<sasl_message_t *>(data));

}


static
void p_service_introduce(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<service_t *>(data));

}


static
void p_user_can_register(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_register_check_t *>(data));

}


static
void p_user_drop(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<myuser_t *>(data));
}


static
void p_user_identify(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<user_t *>(data));

}


static
void p_user_info(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_req_t *>(data));

}


static
void p_user_info_noexist(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_info_noexist_req_t *>(data));

}


static
void p_user_register(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<myuser_t *>(data));

}


static
void p_user_verify_register(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_req_t *>(data));

}


static
void p_user_check_expire(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_expiry_req_t *>(data));

}


static
void p_user_rename(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_rename_t *>(data));

}


static
void p_user_sethost(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<user_t *>(data));

}


static
void p_user_needforce(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_user_needforce_t *>(data));

}


static
void p_myuser_delete(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<myuser_t *>(data));

}


static
void p_metadata_change(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_metadata_change_t *>(data));

}


static
void p_host_request(void *const &data, argv &ret)
{
	//const auto d(reinterpret_cast<hook_host_request_t *>(data));

}


static
void p_channel_pick_successor(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_channel_succession_req_t *>(data));

}


static
void p_channel_succession(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_channel_succession_req_t *>(data));

}


static
void p_grant_channel_access(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<user_t *>(data));
}


static
void p_operserv_info(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<sourceinfo_t *>(data));
}


static
void p_module_load(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_module_load_t *>(data));

}


static
void p_myentity_find(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_myentity_req_t *>(data));

}


static
void p_sasl_may_impersonate(void *const &data, argv &ret)
{
	const auto d(reinterpret_cast<hook_sasl_may_impersonate_t *>(data));

}


static
void handle(const char *const &name,
            void *const data,
            void (&parser)(void *const &, argv &))
noexcept try
{
	if(unlikely(!ecma->hookv))
		return;

	slog(LG_DEBUG, "hook [%s] data: %p\n", name, data);
	ecma->hookv(name, data, parser);
}
catch(const std::exception &e)
{
	slog(LG_ERROR, "Uncaught hook handler exception: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "Uncaught unknown hook handler exception.");
}


#define TARGET_SYM(name) \
target_ ## name ## _


#define TARGET_DEF(name)                        \
static void TARGET_SYM(name)(void *const data)  \
{                                               \
   handle(#name, data, p_ ## name);             \
}


TARGET_DEF(config_purge)
TARGET_DEF(config_ready)
TARGET_DEF(db_write)
TARGET_DEF(db_write_pre_ca)
TARGET_DEF(db_saved)
TARGET_DEF(db_read)
TARGET_DEF(shutdown)
TARGET_DEF(channel_add)
TARGET_DEF(channel_delete)
TARGET_DEF(channel_tschange)
TARGET_DEF(channel_join)
TARGET_DEF(channel_part)
TARGET_DEF(channel_mode)
TARGET_DEF(channel_topic)
TARGET_DEF(channel_can_change_topic)
TARGET_DEF(channel_message)
TARGET_DEF(server_add)
TARGET_DEF(server_eob)
TARGET_DEF(server_delete)
TARGET_DEF(user_add)
TARGET_DEF(user_delete)
TARGET_DEF(user_delete_info)
TARGET_DEF(user_nickchange)
TARGET_DEF(user_away)
TARGET_DEF(user_deoper)
TARGET_DEF(user_oper)
TARGET_DEF(channel_can_register)
TARGET_DEF(channel_drop)
TARGET_DEF(channel_info)
TARGET_DEF(channel_register)
TARGET_DEF(channel_check_expire)
TARGET_DEF(channel_acl_change)
TARGET_DEF(group_drop)
TARGET_DEF(group_register)
TARGET_DEF(nick_can_register)
TARGET_DEF(nick_group)
TARGET_DEF(nick_check)
TARGET_DEF(nick_enforce)
TARGET_DEF(nick_ungroup)
TARGET_DEF(nick_check_expire)
TARGET_DEF(sasl_input)
TARGET_DEF(service_introduce)
TARGET_DEF(user_can_register)
TARGET_DEF(user_drop)
TARGET_DEF(user_identify)
TARGET_DEF(user_info)
TARGET_DEF(user_info_noexist)
TARGET_DEF(user_register)
TARGET_DEF(user_verify_register)
TARGET_DEF(user_check_expire)
TARGET_DEF(user_rename)
TARGET_DEF(user_sethost)
TARGET_DEF(user_needforce)
TARGET_DEF(myuser_delete)
TARGET_DEF(metadata_change)
TARGET_DEF(host_request)
TARGET_DEF(channel_pick_successor)
TARGET_DEF(channel_succession)
TARGET_DEF(grant_channel_access)
TARGET_DEF(operserv_info)
TARGET_DEF(module_load)
TARGET_DEF(myentity_find)
TARGET_DEF(sasl_may_impersonate)


const std::pair<const char *, void (*)(void *)> target[]
{
	{ "config_purge",             TARGET_SYM(config_purge)             },
	{ "config_ready",             TARGET_SYM(config_ready)             },
	{ "db_write",                 TARGET_SYM(db_write)                 },
	{ "db_write_pre_ca",          TARGET_SYM(db_write_pre_ca)          },
	{ "db_saved",                 TARGET_SYM(db_saved)                 },
	{ "db_read",                  TARGET_SYM(db_read)                  },
	{ "shutdown",                 TARGET_SYM(shutdown)                 },
	{ "channel_add",              TARGET_SYM(channel_add)              },
	{ "channel_delete",           TARGET_SYM(channel_delete)           },
	{ "channel_tschange",         TARGET_SYM(channel_tschange)         },
	{ "channel_join",             TARGET_SYM(channel_join)             },
	{ "channel_part",             TARGET_SYM(channel_part)             },
	{ "channel_mode",             TARGET_SYM(channel_mode)             },
	{ "channel_topic",            TARGET_SYM(channel_topic)            },
	{ "channel_can_change_topic", TARGET_SYM(channel_can_change_topic) },
	{ "channel_message",          TARGET_SYM(channel_message)          },
	{ "server_add",               TARGET_SYM(server_add)               },
	{ "server_eob",               TARGET_SYM(server_eob)               },
	{ "server_delete",            TARGET_SYM(server_delete)            },
	{ "user_add",                 TARGET_SYM(user_add)                 },
	{ "user_delete",              TARGET_SYM(user_delete)              },
	{ "user_delete_info",         TARGET_SYM(user_delete_info)         },
	{ "user_nickchange",          TARGET_SYM(user_nickchange)          },
	{ "user_away",                TARGET_SYM(user_away)                },
	{ "user_deoper",              TARGET_SYM(user_deoper)              },
	{ "user_oper",                TARGET_SYM(user_oper)                },
	{ "channel_can_register",     TARGET_SYM(channel_can_register)     },
	{ "channel_drop",             TARGET_SYM(channel_drop)             },
	{ "channel_info",             TARGET_SYM(channel_info)             },
	{ "channel_register",         TARGET_SYM(channel_register)         },
	{ "channel_check_expire",     TARGET_SYM(channel_check_expire)     },
	{ "channel_acl_change",       TARGET_SYM(channel_acl_change)       },
	{ "group_drop",               TARGET_SYM(group_drop)               },
	{ "group_register",           TARGET_SYM(group_register)           },
	{ "nick_can_register",        TARGET_SYM(nick_can_register)        },
	{ "nick_group",               TARGET_SYM(nick_group)               },
	{ "nick_check",               TARGET_SYM(nick_check)               },
	{ "nick_enforce",             TARGET_SYM(nick_enforce)             },
	{ "nick_ungroup",             TARGET_SYM(nick_ungroup)             },
	{ "nick_check_expire",        TARGET_SYM(nick_check_expire)        },
	{ "sasl_input",               TARGET_SYM(sasl_input)               },
	{ "service_introduce",        TARGET_SYM(service_introduce)        },
	{ "user_can_register",        TARGET_SYM(user_can_register)        },
	{ "user_drop",                TARGET_SYM(user_drop)                },
	{ "user_identify",            TARGET_SYM(user_identify)            },
	{ "user_info",                TARGET_SYM(user_info)                },
	{ "user_info_noexist",        TARGET_SYM(user_info_noexist)        },
	{ "user_register",            TARGET_SYM(user_register)            },
	{ "user_verify_register",     TARGET_SYM(user_verify_register)     },
	{ "user_check_expire",        TARGET_SYM(user_check_expire)        },
	{ "user_rename",              TARGET_SYM(user_rename)              },
	{ "user_sethost",             TARGET_SYM(user_sethost)             },
	{ "user_needforce",           TARGET_SYM(user_needforce)           },
	{ "myuser_delete",            TARGET_SYM(myuser_delete)            },
	{ "metadata_change",          TARGET_SYM(metadata_change)          },
	{ "host_request",             TARGET_SYM(host_request)             },
	{ "channel_pick_successor",   TARGET_SYM(channel_pick_successor)   },
	{ "channel_succession",       TARGET_SYM(channel_succession)       },
	{ "grant_channel_access",     TARGET_SYM(grant_channel_access)     },
	{ "operserv_info",            TARGET_SYM(operserv_info)            },
	{ "module_load",              TARGET_SYM(module_load)              },
	{ "myentity_find",            TARGET_SYM(myentity_find)            },
	{ "sasl_may_impersonate",     TARGET_SYM(sasl_may_impersonate)     },
};


static
void install()
{
	for(const auto &t : target)
		hook_add_hook(get<0>(t), get<1>(t));
}


static
void uninstall()
{
	for(const auto &t : target)
		hook_del_hook(get<0>(t), get<1>(t));
}


void module_init(module_t *const m)
noexcept
{
	module_register(m);
	install();
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	uninstall();
}


DECLARE_MODULE_V1
(
	"ecma/hooks",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
