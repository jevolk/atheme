/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


inline
void tokens_mutable(char *const str,
                    const char *const delim,
                    void (*const func)(char *))
{
	char *ctx;
	char *tok = strtok_r(str, delim, &ctx); do
	{
		func(tok);
	}
	while((tok = strtok_r(NULL, delim, &ctx)) != NULL);
}


inline
int tokens_until_mutable(char *const str,
                         const char *const delim,
                         int (*const func)(char *))
{
	char *ctx;
	char *tok = strtok_r(str, delim, &ctx); do
	{
		if(func(tok))
			return 1;
	}
	while((tok = strtok_r(NULL, delim, &ctx)) != NULL);
	return 0;
}


inline
void tokens(const char *const str,
            const char *const delim,
            void (*const func)(const char *))
{
	char sbuf[BUFSIZE];
	const size_t len = strlen(str);
	char *const buf = len >= BUFSIZE? mowgli_alloc(len+1) : sbuf;
	mowgli_strlcpy(buf, str, len+1);
	void wrapper(char *const token)
	{
		func(token);
	}

	tokens_mutable(buf, delim, wrapper);

	if(buf != sbuf)
		mowgli_free(buf);
}


inline
int tokens_until(const char *const str,
                 const char *const delim,
                 int (*const func)(const char *))
{
	char sbuf[BUFSIZE];
	const size_t len = strlen(str);
	char *const buf = len >= BUFSIZE? mowgli_alloc(len+1) : sbuf;
	mowgli_strlcpy(buf, str, len+1);
	int wrapper(char *const token)
	{
		return func(token);
	}

	const int ret = tokens_until_mutable(buf, delim, wrapper);

	if(buf != sbuf)
		mowgli_free(buf);

	return ret;
}
