/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#define MDU_KEY_USERCLOAK              "private:usercloak"
#define MDU_KEY_USERCLOAK_TIME         "private:usercloak-timestamp"
#define MDU_KEY_USERCLOAK_ASSIGNER     "private:usercloak-assigner"

#define GMS_MDU_KEY_SZ                 128
#define GMS_MDU_LIST_SEP               ','
#define MDU_KEY_GMS_PRIVS              MD_NS_GMS"privs"
#define MDU_KEY_GMS_GROUPS             MD_NS_GMS"groups"
const char GMS_MDU_LIST_SEP_STR[]      = { GMS_MDU_LIST_SEP, '\0' };

size_t user_meta_key(const char *key, char *buf, const size_t bufsz);

// User metadata in namespace MD_NS_GMS only.
const char *user_meta_get(const myuser_t *user, const char *key);
void user_meta_set(myuser_t *user, const char *key, const char *val);
void user_meta_del(myuser_t *user, const char *key);

// Algorithms over the GMS metadata.
void user_meta_foreach(const myuser_t *user, void (*func)(const char *, const char *));
int user_meta_until(const myuser_t *user, int (*func)(const char *, const char *));
size_t user_meta_count(const myuser_t *user);

// Algorithms over metadata with list values
void user_meta_list_foreach(const myuser_t *user, const char *key, void (*func)(const char *));
int user_meta_list_until(const myuser_t *user, const char *key, int (*func)(const char *));
size_t user_meta_list_count(const myuser_t *user, const char *key);
int user_meta_list_has(const myuser_t *user, const char *key, const char *val);
int user_meta_list_remove_if(myuser_t *user, const char *key, int (*func)(const char *));
int user_meta_list_remove(myuser_t *user, const char *key, const char *val);
int user_meta_list_append(myuser_t *user, const char *key, const char *val);

// Controls the user's cloak (not metadata in GMS namespace)
const char *user_cloak_get_str(const myuser_t *user);
void user_cloak_set_str(myuser_t *user, const char *str);
void user_cloak_unset(myuser_t *user);
void user_cloak_set(myuser_t *user, const char *str);

// Convenience priv check and functors for cmd_t
int user_priv(const myuser_t *user, const char *priv);
int user_priv_admin(const user_t *u);
int user_priv_staff(const user_t *u);
int user_priv_auth(const user_t *u);
int user_priv_none(const user_t *u);

user_t *user_get_user(myuser_t *user);                           // Get the last active user_t
int user_notice_chan(myuser_t *user, const char *msg);
int user_notice_last(myuser_t *user, const char *msg);
int user_notice(myuser_t *user, const char *msg);
int user_memo(myuser_t *user, const char *msg);
int user_send(myuser_t *user, const char *msg);


inline
int user_send(myuser_t *const user,
              const char *const msg)
{
	int ret = 0;
	if(user_meta_get(user, "send_memos"))
		ret += user_memo(user, msg);

	if(user_meta_get(user, "send_notice_all"))
		ret += user_notice(user, msg);

	if(user_meta_get(user, "send_notice_last"))
		ret += user_notice_last(user, msg);

	// try one last time to get something through
	if(!ret)
		user_notice_last(user, msg);

	return ret;
}


inline
int user_memo(myuser_t *const user,
              const char *const m)
{
	msg(myservice->me->nick, "memoserv", "SEND %s %s", entity(user)->name, m);
	//note: no readback
	return 1;
}


inline
int user_notice(myuser_t *const user,
                const char *const msg)
{
	if(!user->logins.count)
		return 0;

	myuser_notice(myservice->me->nick, user, "%s", msg);
	return 1;
}


inline
int user_notice_last(myuser_t *const user,
                     const char *const msg)
{
	user_t *const u = user_get_user(user);
	if(!u)
		return 0;

	notice_user_sts(myservice->me, u, msg);
	return 1;
}


inline
int user_notice_chan(myuser_t *const user,
                     const char *const msg)
{
	return 0;
}


inline
user_t *user_get_user(myuser_t *const user)
{
	mowgli_node_t *node;
	user_t *ret = NULL, *login = NULL;
	MOWGLI_LIST_FOREACH(node, user->logins.head)
	{
		login = node->data;
		if(!ret || login->lastmsg > ret->lastmsg)
			ret = login;
	}

	return ret;
}


inline
int user_priv_none(const user_t *u)
{
	return 1;
}


inline
int user_priv_auth(const user_t *u)
{
	return u->myuser != NULL;
}


inline
int user_priv_staff(const user_t *u)
{
	return user_priv_admin(u)?   1:
	       u->myuser?            user_priv(u->myuser, "staff"):
	                             0;
}


inline
int user_priv_admin(const user_t *u)
{
	return u->myuser && u->myuser->soper? 1:
	       u->myuser?                     user_priv(u->myuser, "admin"):
	                                      0;
}


inline
int user_priv(const myuser_t *const user,
              const char *const priv)
{
	return user_meta_list_has(user, "privs", priv);
}


inline
void user_cloak_unset(myuser_t *const user)
{
	metadata_delete(user, MDU_KEY_USERCLOAK);
	metadata_delete(user, MDU_KEY_USERCLOAK_TIME);
	metadata_delete(user, MDU_KEY_USERCLOAK_ASSIGNER);

	mowgli_node_t *n;
	MOWGLI_ITER_FOREACH(n, user->logins.head)
	{
		user_t *const u = n->data;
		if(strcmp(u->vhost, u->host) == 0)
			continue;

		user_sethost(myservice->me, u, u->host);
	}
}


inline
void user_cloak_set(myuser_t *const user,
                    const char *const cloak)
{
	user_cloak_set_str(user, cloak);

	mowgli_node_t *n;
	MOWGLI_ITER_FOREACH(n, user->logins.head)
	{
		user_t *const u = n->data;
		if(strcmp(u->vhost, cloak) == 0)
			continue;

		user_sethost(myservice->me, u, cloak);
	}
}


inline
void user_cloak_set_str(myuser_t *const user,
                        const char *const cloak)
{
	char timestr[16];
	snprintf(timestr, sizeof(timestr), "%ld", CURRTIME);

	metadata_add(user, MDU_KEY_USERCLOAK, cloak);
	metadata_add(user, MDU_KEY_USERCLOAK_TIME, timestr);
	metadata_add(user, MDU_KEY_USERCLOAK_ASSIGNER, GMS_NAME);
}


inline
const char *user_cloak_get_str(const myuser_t *const user)
{
	const metadata_t *const md = metadata_find((myuser_t *)user, MDU_KEY_USERCLOAK);
	if(!md)
		return NULL;

	return md->value;
}


inline
int user_meta_list_append(myuser_t *const user,
                          const char *const key,
                          const char *const val)
{
	const char *const str = user_meta_get(user, key);
	char buf[BUFSIZE];
	if(str)
		strvec_tokenize(str, GMS_MDU_LIST_SEP_STR, buf, sizeof(buf));
	else
		strvec_init(buf);

	if(strvec_find(buf, val))
	{
		gmserr = "Already exists in list.";
		return 0;
	}

	if(!strvec_append(buf, sizeof(buf), val))
	{
		gmserr = "Not enough space in list buffer for token.";
		return 0;
	}

	strvec_join_inplace(buf, GMS_MDU_LIST_SEP);
	user_meta_set(user, key, buf);
	return 1;
}


inline
int user_meta_list_remove(myuser_t *const user,
                          const char *const key,
                          const char *const val)
{
	int match(const char *const token)
	{
		return irccasecmp(val, token) == 0;
	}

	return user_meta_list_remove_if(user, key, match);
}


inline
int user_meta_list_remove_if(myuser_t *const user,
                             const char *const key,
                             int (*const func)(const char *))
{
	const char *const str = user_meta_get(user, key);
	if(!str)
		return 0;

	char buf[BUFSIZE];
	strvec_tokenize(str, GMS_MDU_LIST_SEP_STR, buf, sizeof(buf));
	if(!strvec_remove_if(buf, func))
		return 0;

	if(!strvec_join_inplace(buf, GMS_MDU_LIST_SEP))
	{
		user_meta_del(user, key);
		return 1;
	}

	user_meta_set(user, key, buf);
	return 1;
}


inline
int user_meta_list_has(const myuser_t *const user,
                       const char *const key,
                       const char *const val)
{
	int match(const char *const str)
	{
		return irccasecmp(str, val) == 0;
	}

	return user_meta_list_until(user, key, match);
}


inline
size_t user_meta_list_count(const myuser_t *const user,
                            const char *const key)
{
	size_t ret = 0;
	void counter(const char *const str)
	{
		ret++;
	}

	user_meta_list_foreach(user, key, counter);
	return ret;
}


inline
int user_meta_list_until(const myuser_t *const user,
                         const char *const key,
                         int (*const func)(const char *))
{
	const char *const str = user_meta_get(user, key);
	if(!str)
		return 0;

	return tokens_until(str, GMS_MDU_LIST_SEP_STR, func);
}


inline
void user_meta_list_foreach(const myuser_t *const user,
                            const char *const key,
                            void (*const func)(const char *))
{
	const char *const str = user_meta_get(user, key);
	if(!str)
		return;

	tokens(str, GMS_MDU_LIST_SEP_STR, func);
}


inline
size_t user_meta_count(const myuser_t *const user)
{
	size_t ret = 0;
	void counter(const char *const k, const char *const v)
	{
		ret++;
	}

	user_meta_foreach(user, counter);
	return ret;
}


inline
int user_meta_until(const myuser_t *const user,
                    int (*func)(const char *, const char *))
{
	int gms_filter(const char *const key, const char *const val)
	{
		if(strncmp(key, MD_NS_GMS, strlen(MD_NS_GMS)) == 0)
			return func(key + strlen(MD_NS_GMS), val);

		return 0;
	}

	return metadata_until(object(user), gms_filter);
}


inline
void user_meta_foreach(const myuser_t *const user,
                       void (*func)(const char *, const char *))
{
	void gms_filter(const char *const key, const char *const val)
	{
		if(strncmp(key, MD_NS_GMS, strlen(MD_NS_GMS)) == 0)
			func(key + strlen(MD_NS_GMS), val);
	}

	metadata_foreach(object(user), gms_filter);
}


inline
void user_meta_del(myuser_t *const user,
                   const char *const key)
{
	char k[GMS_MDU_KEY_SZ];
	user_meta_key(key, k, sizeof(k));
	metadata_delete(user, k);
}


inline
void user_meta_set(myuser_t *const user,
                   const char *const key,
                   const char *const val)
{
	char k[GMS_MDU_KEY_SZ];
	user_meta_key(key, k, sizeof(k));
	metadata_add(user, k, val);
}


inline
const char *user_meta_get(const myuser_t *const user,
                          const char *const key)
{
	char k[GMS_MDU_KEY_SZ];
	user_meta_key(key, k, sizeof(k));
	const metadata_t *const md = metadata_find(object(user), k);
	return md? md->value : NULL;
}


inline
size_t user_meta_key(const char *const key,
                     char *const buf, const size_t bufsz)
{
	mowgli_strlcpy(buf, MD_NS_GMS, bufsz);
	return mowgli_strlcat(buf, key, bufsz);
}
