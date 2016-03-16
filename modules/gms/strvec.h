/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


inline
const char *strvec_end(const char *vec)
{
	for(; *vec; vec += strlen(vec)+1);
	return vec;
}


inline
char *strvec_end_mutable(char *vec)
{
	for(; *vec; vec += strlen(vec)+1);
	return vec;
}


inline
const char *strvec_at(const char *vec,
                      const size_t idx)
{
	size_t i = 0;
	for(; *vec && i < idx; vec += strlen(vec)+1, i++);
	return vec;
}


inline
char *strvec_at_mutable(char *vec,
                        const size_t idx)
{
	size_t i = 0;
	for(; *vec && i < idx; vec += strlen(vec)+1, i++);
	return vec;
}


inline
void strvec_foreach(const char *vec,
                    void (*const func)(const char *))
{
	for(; *vec; vec += strlen(vec)+1)
		func(vec);
}


inline
void strvec_foreach_mutable(char *vec,
                            void (*const func)(char *))
{
	for(size_t l = 0; *vec; vec += l)
	{
		l = strlen(vec) + 1;
		func(vec);
	}
}


inline
int strvec_while(const char *vec,
                 int (*const func)(const char *))
{
	for(; *vec; vec += strlen(vec)+1)
		if(!func(vec))
			return 0;

	return 1;
}


inline
int strvec_while_mutable(char *vec,
                         int (*const func)(char *))
{
	for(size_t l = 0; *vec; vec += l)
	{
		l = strlen(vec) + 1;
		if(!func(vec))
			return 0;
	}

	return 1;
}


inline
int strvec_until(const char *vec,
                 int (*const func)(const char *))
{
	for(; *vec; vec += strlen(vec)+1)
		if(func(vec))
			return 1;

	return 0;
}


inline
int strvec_until_mutable(char *vec,
                         int (*const func)(char *))
{
	for(size_t l = 0; *vec; vec += l)
	{
		l = strlen(vec) + 1;
		if(func(vec))
			return 1;
	}

	return 0;
}


inline
size_t strvec_len(const char *vec)
{
	size_t ret = 1;
	while(*vec)
	{
		const size_t len = strlen(vec);
		vec += len + 1;
		ret += len + 1;
	}

	return ret;
}


inline
size_t strvec_count(const char *const vec)
{
	size_t ret = 0;
	void count(const char *const token)
	{
		ret++;
	}

	strvec_foreach(vec, count);
	return ret;
}


inline
size_t strvec_count_if(const char *const vec,
                       int (*const func)(const char *))
{
	size_t ret = 0;
	void count(const char *const token)
	{
		ret += func(token) != 0;
	}

	strvec_foreach(vec, count);
	return ret;
}


inline
char *strvec_append(char *vec,
                    const size_t buf_max,
                    const char *const token)
{
	char *ret = strvec_end_mutable(vec);
	const size_t rem = buf_max - (ret - vec);

	if(rem <= strlen(token) + 2)
		return NULL;

	const size_t cpy = mowgli_strlcpy(ret, token, rem);
	ret[cpy+1] = '\0';
	return ret;
}


inline
size_t strvec_find(const char *vec,
                   const char *const token)
{
	size_t ret = 0;
	int find(const char *const t)
	{
		if(strcmp(t,token) == 0)
			return 0;

		ret++;
		return 1;
	}

	strvec_while(vec, find);
	return ret;
}


inline
int strvec_remove_at(char *vec,
                     const size_t index)
{
	char *const ptr = strvec_at_mutable(vec, index);
	if(ptr[0] == '\0')
		return 0;

	const size_t len = strlen(ptr) + 1;
	const char *const src = ptr + len;
	memmove(ptr,src,len);
	return 1;
}


inline
size_t strvec_remove_if(char *vec,
                        int (*func)(const char *))
{
	size_t ret = 0;
	const ssize_t count = strvec_count(vec);
	for(ssize_t i = count - 1; i > -1; i--)
	{
		if(func(strvec_at(vec, i)))
		{
			strvec_remove_at(vec, i);
			ret++;
		}
	}

	return ret;
}


inline
int strvec_remove(char *vec,
                  const char *const token)
{
	const size_t idx = strvec_find(vec, token);
	return strvec_remove_at(vec, idx);
}


inline
size_t strvec_join_inplace(char *const vec,
                           const char delim)
{
	size_t ret = 0;
	void make_delim(char *const token)
	{
		const size_t len = strlen(token);
		token[len] = delim;
		ret += len + 1;
	}

	strvec_foreach_mutable(vec, make_delim);

	if(ret)
		vec[--ret] = '\0';

	return ret;
}


inline
size_t strvec_join_copy(const char *const vec,
                        const char *const delim,
                        char *const buf,
                        const size_t buf_max)
{
	size_t ret = 0;
	void concat(const char *const token)
	{
		ret = mowgli_strlcat(buf, token, buf_max);
		ret = mowgli_strlcat(buf, delim, buf_max);
	}

	strvec_foreach(vec, concat);

	if(ret > 0)
		buf[ret-1] = '\0';

	return ret;
}


inline
int strvec_tokenize(const char *const str,
                    const char *const delim,
                    char *const vec_buf,
                    const size_t buf_max)
{
	size_t len = 0;
	int i = 0, ret = 0;
	void append(char *const token)
	{
		if(i++ > ret)
			return;

		const size_t toklen = strlen(token);
		if(len + toklen + 2 >= buf_max)
			return;

		len += toklen + 1;
		ret++;
	}

	const size_t copied = mowgli_strlcpy(vec_buf, str, buf_max);
	tokens_mutable(vec_buf, delim, append);
	vec_buf[len] = '\0';
	return ret;
}


inline
void strvec_clear(char *vec)
{
	vec[0] = vec[1] = '\0';
}


inline
void strvec_init(char *vec)
{
	vec[0] = vec[1] = '\0';
}
