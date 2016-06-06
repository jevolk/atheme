/*
 * Copyright (C) Jason Volk 2016
 *
 * Rights to this code may be determined by the following organizations:
 * - Atheme Development Group
 * - Freenode
 */


inline
void sendto_realops_snomask(const char *snobuf, const char *fmt, ...) PRINTFLIKE(2, 3);
void sendto_realops_snomask(const char *const snobuf,
                            const char *const fmt,
                            ...)
{
	va_list ap;
	va_start(ap, fmt);
	char buf[BUFSIZE];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	sts(":%s ENCAP * SNOTE %s :%s", ME, snobuf, buf);
	va_end(ap);
}


inline
ssize_t util_parv_to_kv(const int parc,
                        char **parv,
                        const char **const key,    // return (must be parc+1)
                        const char **const val)    // return (must be parc+1)
{
	ssize_t i = 0;
	for(; i < parc; i++)
	{
		char *ctx;
		key[i] = strtok_r(parv[i], "=", &ctx);
		val[i] = strtok_r(NULL, "=", &ctx);
		if(!key[i] || !val[i])
			return -1;
	}

	key[i] = NULL;
	val[i] = NULL;
	return i;
}


static
void debug_result(const PGresult *const r,
                  void (*const liner)(const char *line, void *priv),
                  void *const priv)
{
	const PQprintOpt opt =
	{
		.header = true,
		.align = true,
		.standard = false,
		.html3 = false,
		.expanded = false,
		.pager = false,
		.fieldSep = (char *)"|",
		.tableOpt = (char *)"",
		.caption = (char *)"",
	};

	static char buf[1024 * 1024];
	memset(buf, 0x0, sizeof(buf));
	FILE *f = fmemopen(buf, sizeof(buf), "w+");
	PQprint(f, r, &opt);
	fseek(f, 0, SEEK_SET);
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while((read = getline(&line, &len, f)) != -1)
		liner(line, priv);

	free(line);
	fclose(f);
}
