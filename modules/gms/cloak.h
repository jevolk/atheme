/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#define GMS_CLOAK_GROUP_SEP   '/'
#define GMS_CLOAK_ROLE_SEP    '.'


int cloak_to_strvec(const char *cloak, char *vec_buf, const size_t buf_max);
void cloak_foreach(const char *cloak, void (*func)(const char *, const char *));
int cloak_until(const char *cloak, int (*func)(const char *, const char *));
int cloak_has_keyval(const char *cloak, const char *key, const char *val);
int cloak_has_val(const char *cloak, const char *val);
int cloak_has_key(const char *cloak, const char *key);

size_t cloak_append(char *cloak, const size_t max, const char *key, const char *val);


inline
size_t cloak_append(char *const cloak,
                    const size_t max,
                    const char *const key,
                    const char *const val)
{
	const char role_sep[]  = { GMS_CLOAK_ROLE_SEP, '\0'  };
	const char group_sep[] = { GMS_CLOAK_GROUP_SEP, '\0' };

	mowgli_strlcat(cloak, key, max);
	mowgli_strlcat(cloak, role_sep, max);
	mowgli_strlcat(cloak, val, max);
	mowgli_strlcat(cloak, group_sep, max);
}


inline
int cloak_has_key(const char *const cloak,
                  const char *const key)
{
	int check(const char *const k, const char *const v)
	{
		return irccasecmp(key,k) == 0;
	}

	return cloak_until(cloak,check);
}


inline
int cloak_has_val(const char *const cloak,
                  const char *const val)
{
	int check(const char *const k, const char *const v)
	{
		return v != NULL && irccasecmp(val,v) == 0;
	}

	return cloak_until(cloak,check);
}


inline
int cloak_has_keyval(const char *const cloak,
                     const char *const key,
                     const char *const val)
{
	int check(const char *const k, const char *const v)
	{
		return irccasecmp(key,k) != 0?     0:
		       val == NULL && v == NULL?   1:
		       irccasecmp(val,v) == 0?     1:
		                                   0;
	}

	return cloak_until(cloak,check);
}


inline
int cloak_until(const char *const cloak,
                int (*const func)(const char *, const char *))
{
	int parse(char *const token)
	{
		char *const val = strchr(token, GMS_CLOAK_ROLE_SEP);
		if(!val)
			return func(token,val);

		*val = '\0';
		const int ret = func(token,val+1);
		*val = GMS_CLOAK_ROLE_SEP;
		return ret;
	}

	char buf[BUFSIZE];
	cloak_to_strvec(cloak,buf,sizeof(buf));
	return strvec_until_mutable(buf,parse);
}


inline
void cloak_foreach(const char *const cloak,
                   void (*const func)(const char *, const char *))
{
	void parse(char *const token)
	{
		char *const val = strchr(token, GMS_CLOAK_ROLE_SEP);
		if(!val)
		{
			func(token,val);
			return;
		}

		*val = '\0';
		func(token,val+1);
		*val = GMS_CLOAK_ROLE_SEP;
	}

	char buf[BUFSIZE];
	cloak_to_strvec(cloak,buf,sizeof(buf));
	strvec_foreach_mutable(buf,parse);
}


inline
int cloak_to_strvec(const char *const cloak,
                    char *const vec_buf,
                    const size_t buf_max)
{
	static const char delim[] = { GMS_CLOAK_GROUP_SEP, '\0' };

	return strvec_tokenize(cloak, delim, vec_buf, buf_max);
}
