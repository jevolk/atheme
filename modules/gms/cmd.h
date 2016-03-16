/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


typedef struct cmd_t
{
	const char *name;
	int (*handler)(struct cmd_t *, sourceinfo_t *, int, char *[]);
	int minargs;
	const char *usage;
	int (*privs_check)(const user_t *);
	struct cmd_t *child[32];
	struct cmd_t *parent;
	void *data;
}
cmd_t;

static __thread const char *cmd_tokens[16];
static __thread int cmd_depth;

void *cmd_data(cmd_t *const cmd);
cmd_t *cmd_parent(cmd_t *const cmd);
void *cmd_parent_data(cmd_t *const cmd);
void cmd_set_data(cmd_t *const cmd, void *const data);
size_t cmd_children(const cmd_t *const cmd);
static const char *cmd_path_str(const char **elems, const int depth);
static const char *cmd_path(const cmd_t *cmd);
void cmd_show_children(cmd_t *cmd, sourceinfo_t *si);
cmd_t *cmd_get_child(cmd_t *const cmd, const char *const name);
static int cmd_call(cmd_t *const cmd, sourceinfo_t *const si, int parc, char *parv[]);


static
int cmd_call(cmd_t *const cmd, sourceinfo_t *const si, int parc, char *parv[])
{
	void undepth(const void *const f)
	{
		cmd_depth--;
	}

	if(cmd_depth >= 15)
	{
		command_fail(si, fault_badparams, "internal error: recursion limit exceeded.");
		return -1;
	}

	// Please avoid tangential longjmps out of this function.
	const void *const unwind scope(undepth);
	cmd_tokens[cmd_depth++] = cmd->name;

	if(cmd->privs_check && !cmd->privs_check(si->su))
	{
		const char *const p = cmd_path(cmd);
		command_fail(si, fault_noprivs, "Access denied to command %s.", p);
		return -1;
	}

	if(parc < cmd->minargs)
	{
		const char *const p = cmd_path(cmd);
		command_fail(si, fault_needmoreparams, "Insufficient parameters for: %s. Usage: %s %s", p, p, cmd->usage);
		cmd_show_children(cmd, si);
		return -1;
	}

	if(cmd->handler)
	{
		if(!cmd->handler(cmd, si, parc, parv))
		{
			const char *const p = cmd_path(cmd);
			const char *const str = gmserr?: "internal error.";
			command_fail(si, fault_badparams, "error: %s: %s", p, str);
			gmserr = NULL;
			return 0;
		}
	}

	if(!cmd_children(cmd))
		return 1;

	// If this handler used a parameter to store data for the child,
	// that parameter is not seen by the child.
	if(cmd_data(cmd) && parc > 1)
	{
		parc--;
		parv++;
	}

	cmd_t *const child = cmd_get_child(cmd, parv[0]);
	if(!child)
	{
		const char *const p = cmd_path(cmd);
		command_fail(si, fault_badparams, "%s: command not found.", p);
		cmd_show_children(cmd, si);
		return -1;
	}

	child->parent = cmd;
	parc--;
	parv++;

	return cmd_call(child, si, parc, parv);
}


inline
void cmd_show_children(cmd_t *const cmd,
                       sourceinfo_t *const si)
{
	char buf[BUFSIZE]; buf[0] = '\0';
	for(size_t i = 0; i < 32; i++)
	{
		cmd_t *const child = cmd->child[i];
		if(!child || !child->name)
			continue;

		if(child->privs_check && !child->privs_check(si->su))
			continue;

		mowgli_strlcat(buf, child->name, sizeof(buf));
		mowgli_strlcat(buf, ", ", sizeof(buf));
	}

	if(strlen(buf))
		command_success_nodata(si, _("commands: \2%s\2"), buf);
}


static
const char *cmd_path(const cmd_t *const cmd)
{
	return cmd_path_str(cmd_tokens, cmd_depth);
}


static
const char *cmd_path_str(const char **elems,
                         const int depth)
{
	static __thread char ret[BUFSIZE];

	ret[0] = '\0';
	for(int i = 0; i < depth && i < 16; i++)
	{
		mowgli_strlcat(ret, elems[i], sizeof(ret));
		if(i + 1 < depth)
			mowgli_strlcat(ret, " ", sizeof(ret));
	}

	return ret;
}


inline
cmd_t *cmd_get_child(cmd_t *const cmd,
                     const char *const name)
{
	for(size_t i = 0; i < 32; i++)
	{
		cmd_t *const child = cmd->child[i];
		if(child && child->name && irccasecmp(name, child->name) == 0)
			return child;
	}

	return NULL;
}


inline
size_t cmd_children(const cmd_t *const cmd)
{
	size_t ret = 0;
	for(size_t i = 0; i < 32; i++)
		if(cmd->child[i])
			ret++;

	return ret;
}


inline
void cmd_set_data(cmd_t *const cmd,
                  void *const data)
{
	cmd->data = data;
}


inline
void *cmd_parent_data(cmd_t *const cmd)
{
	return cmd_parent(cmd)? cmd_data(cmd_parent(cmd)) : NULL;
}


inline
cmd_t *cmd_parent(cmd_t *const cmd)
{
	return cmd->parent;
}


inline
void *cmd_data(cmd_t *const cmd)
{
	return cmd->data;
}
