/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


#define scope(fun)  __attribute__((__cleanup__(fun)))


static
const char *gmserr_prepend(const char *const msg)
{
	static char buf[2][BUFSIZE];
	static uint8_t selector;
	selector = !selector;

	buf[selector][0] = '\0';
	mowgli_strlcat(buf[selector], msg, sizeof(buf[selector]));

	if(gmserr)
		mowgli_strlcat(buf[selector], gmserr, sizeof(buf[selector]));

	gmserr = buf[selector];
	return gmserr;
}


inline
int metadata_until(const object_t *const target,
                   int (*const func)(const char *key, const char *val))
{
	const void *md;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(md, &state, object(target)->metadata)
	{
		const char *const key = ((const metadata_t *)md)->name;
		const char *const val = ((const metadata_t *)md)->value;
		if(func(key,val))
			return 1;
	}

	return 0;
}


inline
void metadata_foreach(const object_t *const target,
                      void (*const func)(const char *key, const char *val))
{
	const void *md;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(md, &state, object(target)->metadata)
	{
		const char *const key = ((const metadata_t *)md)->name;
		const char *const val = ((const metadata_t *)md)->value;
		func(key,val);
	}
}


inline
void config_defaults_foreach(void (*const func)(const char *, const char *))
{
	const size_t elems = sizeof(config_defaults) / sizeof(const char *);
	for(size_t i = 0; i < elems; i += 2)
	{
		const char *const key = config_defaults[i];
		const char *const val = config_defaults[i+1];
		func(key, val);
	}
}

