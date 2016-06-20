/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "ecma.h"
#include "module.h"


function::generator shell
{ "shell", {"argv"},
R"(
	irc.msg('jzk', "shell spawned with pid: " + getpid());

	let msg = yield read(stdin);
	irc.msg('jzk', 'you sent ' + msg);

	return null;
)"};


void ecma_ps(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	using namespace v8;

	command_success_nodata(si, "\2%-15s %6s %s %10s %8s %-15s\2"
	                      ,"USER"
	                      ,"PID"
	                      ,"S"
	                      ,"TIME"
	                      ,"SIZE"
	                      ,"TTY"
	                      );
/*
	std::for_each(begin(tasks), end(tasks), [&si]
	(const auto &p)
	{
		const auto &task(*p.second);
		const auto owner(task.get_owner()? entity(task.get_owner())->name : "kernel");

		char tty[32];
		snprintf(tty, sizeof(tty), "%s%s",
		         task.tty? task.tty->nick : "-",
		         task.tty && (task.flags & task.FOREGROUND)? "*" : "");

		command_success_nodata(si, "%-15s %6d %c %3ld.%06ld %8zu %-15s"
		                      ,owner
		                      ,task.pid
		                      ,task.status
		                      ,task.cpu_time / (1000 * 1000)
		                      ,task.cpu_time % (1000 * 1000)
		                      ,task.context_size()
		                      ,tty
		                      );
	});
*/

	command_success_nodata(si, "*** \2End of ps\2 ***");
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_ps =
{
    "PS",
    N_(N_("List tasks.")),
    AC_AUTHENTICATED,
    1,
    ecma_ps,
    { NULL, NULL }
};


const char *const halp = R"(
The first line of input spawns a new foreground process. If the execution
of this line does not return a result, or returns an 'undefined' result,
your process enters the HALT state and waits for additional input. If additional
input does not compile, the line is discarded. If a line returns a valid result,
your process enters the ACCEPT state. The result is normally returned to
the parent process, but the foreground process continues. If an exception is
unhandled, the process enters the EXCEPT state: the last line is discarded, but
the runtime state is not rolled back; this is only true for the foreground
processes, otherwise uncaught exceptions terminate.
)";


void ecma_help(sourceinfo_t *si, int parc, char *parv[])
noexcept try
{
	if(!parc)
	{
		command_success_nodata(si, "****** \2 ECMAScript Interface \2 ******");
		command_success_nodata(si, "******     \2TeleType Help\2      ******");
		command_success_nodata(si, " ");
		command_success_nodata(si, "\2All input is parsed as ECMAScript.\2");
		command_success_nodata(si, "With the exception of TTY assertions prefixed by \2#\2");
		command_success_nodata(si, " ");

		command_success_tty(si, 64, "%s", halp);

		command_success_nodata(si," ");
		command_success_nodata(si, _("For more information on an assertion, type:"));
		command_success_nodata(si, "\2/%s%s #help <command>\2", (ircd->uses_rcommand == false) ? "msg " : "", si->service->nick);
		command_success_nodata(si, " ");

		command_help(si, si->service->commands);

		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}
	else help_display_as_subcmd(si, si->service, NULL, parv[0], si->service->commands);
}
catch(const std::exception &e)
{
	command_fail(si, fault_badparams, "Internal error: %s", e.what());
	slog(LG_ERROR, "ECMA: Unhandled exception: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_help =
{
	"HELP",
	N_(N_("Displays contextual help information.")),
	AC_NONE,
	1,
	ecma_help,
	{ NULL, NULL }
};


void handle_command(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	if(parc < 2)
		return;

	char *const text = parv[1];
	if(!text)
		return;

	const auto len(strlen(text));
	if(!len)
		return;

	const struct isolate_scope isolate_scope;
	const v8::HandleScope handle_scope(isolate());

	// Entry for out-of-band commands that begin with #
	if(text[0] == '#' || text[0] == '\001')
	{
		char *ctx, *const cmd(strtok_r(text, " ", &ctx));
		if(!cmd || !cmd[0])
			return;

		char *const remain(strtok_r(NULL, "", &ctx));
		if(text[0] == '\001')
		{
			if(len > 1)
				handle_ctcp_common(si, cmd, remain);

			return;
		}

		command_exec_split(myservice, si, cmd+1, remain, myservice->commands);
		return;
	}

	ecma->input(si, text);
}
catch(const std::exception &e)
{
	command_fail(si, fault_badparams, "Internal error: %s", e.what());
	slog(LG_ERROR, "ECMA: Unhandled exception: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}



void init(module_t *const m)
noexcept
{
	module_register(m);
	ecma->add(&shell);
	service_bind_command(myservice, &cmd_help);
	service_bind_command(myservice, &cmd_ps);
	myservice->handler = handle_command;

//	const isolate_scope isolate_scope;
//	const handle_scope handle_scope;
}


void fini(module_unload_intent_t)
noexcept
{
	myservice->handler = nullptr;
	service_unbind_command(myservice, &cmd_ps);
	service_unbind_command(myservice, &cmd_help);
	ecma->del(&shell);
}


DECLARE_MODULE_V1
(
	"ecma/tty",
	MODULE_UNLOAD_CAPABILITY_OK,
	init,
	fini,
	PACKAGE_STRING,
	"jzk"
);


/*
	service_bind_command(myservice, &cmd_kill);
	service_bind_command(myservice, &cmd_intr);
	service_bind_command(myservice, &cmd_quit);
	service_bind_command(myservice, &cmd_stop);
	service_bind_command(myservice, &cmd_cont);
	service_bind_command(myservice, &cmd_pop);
	service_bind_command(myservice, &cmd_bg);
	service_bind_command(myservice, &cmd_fg);
*/

/*
	service_unbind_command(myservice, &cmd_fg);
	service_unbind_command(myservice, &cmd_bg);
	service_unbind_command(myservice, &cmd_pop);
	service_unbind_command(myservice, &cmd_cont);
	service_unbind_command(myservice, &cmd_stop);
	service_unbind_command(myservice, &cmd_quit);
	service_unbind_command(myservice, &cmd_intr);
	service_unbind_command(myservice, &cmd_kill);
*/

/*
static
void handle_ttyin(sourceinfo_t *const &si,
                  const char *const &input,
                  dasein &dasein)
try
{
	dasein.script.push(input);
	dasein.script.compile(dasein);
	if(dasein.status != dasein.STOPPED)
		ecma->schedule(dasein);
}
catch(const exception &e)
{
	command_success_nodata(si, "\2\3%02d%s\x0f", uint(colors::FG::RED), e.what());
	dasein.script.pop();
}


static
void run(sourceinfo_t *const &si,
         const char *const &input)
try
{
	const std::shared_ptr<dasein> d(ecma->vorhanden.spawn(si->smu));
	ecma->vorhanden.set_foreground(si->su, *d);
	d->script.push(input);
	d->script.compile(d->context());
	ecma->schedule(*d);
}
catch(const exception &e)
{
	command_success_nodata(si, "\2\3%02d%s\x0f", uint(colors::FG::RED), e.what());
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(dasein)
		dasein->script.pop();
}


static
void eval(sourceinfo_t *const &si,
          const char *const &input)
noexcept try
{
	using namespace v8;

	if(!si->smu)
	{
		command_fail(si, fault_noprivs, "You are not logged in.");
		return;
	}

	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(dasein)
	{
		handle_ttyin(si, input, *dasein);
		return;
	}

	run(si, input);
}
catch(const std::exception &e)
{
	command_success_nodata(si, "(Internal Error): %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}


void ecma_bg(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
	{
		command_success_nodata(si, "No foreground job.");
		return;
	}

	dasein->flags &= ~dasein::FOREGROUND;
	command_success_nodata(si, "[%d]+ @%zu &",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_bg =
{
    "BG",
    N_(N_("Send a process to the background.")),
    AC_AUTHENTICATED,
    0,
    ecma_bg,
    { NULL, NULL }
};


void ecma_fg(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	if(parc < 1 || (!atol(parv[0]) && parv[0][0] != '0'))
	{
		const auto dasein(ecma->vorhanden.foreground(si->su));
		if(!dasein)
		{
			command_success_nodata(si, "No foreground process.");
			return;
		}

		if(parc < 1)
		{
			command_success_nodata(si, "[%d] @%zu",
			                       dasein->pid,
			                       dasein->script.lines());
			return;
		}

		switch(hash(parv[0]))
		{
			case hash("dump"):
			{
				size_t i(0);
				for(const auto &line : dasein->script.code)
					command_success_nodata(si, "[%d]@%-3zu %s",
					                       dasein->pid,
					                       ++i,
					                       string(local(line)));

				return;
			}

			case hash("pop"):
			{
				dasein->script.pop();
				if(!dasein->script.lines())
					return;

				auto &line(dasein->script.code.back());
				command_success_nodata(si, "[%d]@%-3zu@ %s",
				                       dasein->pid,
				                       dasein->script.lines(),
				                       string(local(line)));
				return;
			}

			default:
			{
				command_success_nodata(si, "Invalid argument");
				return;
			}
		}

		return;
	}

	const auto pid(atol(parv[0]));
	if(!pid && (parv[0][0] != '0' || !si->smu->soper))
	{
		command_fail(si, fault_badparams, "Invalid PID.");
		return;
	}

	const auto dasein(ecma->vorhanden[pid]);
	if(!dasein)
	{
		command_success_nodata(si, "No foreground job, specify a PID.");
		return;
	}

	const auto existing(ecma->vorhanden.foreground(si->su));
	if(existing)
		command_success_nodata(si, "[%d]+ @%zu &",
		                       existing->pid,
		                       existing->script.lines());

	ecma->vorhanden.set_foreground(si->su, *dasein);
	command_success_nodata(si, "[%d] @%zu",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::out_of_range &e)
{
	command_success_nodata(si, "No process with that PID exists..");
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_fg =
{
	"FG",
	N_(N_("Send a process to the foreground.")),
	AC_AUTHENTICATED,
	1,
	ecma_fg,
	{ NULL, NULL }
};


void ecma_pop(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
		return;

	dasein->script.pop();
	if(!dasein->script.lines())
		return;

	auto &line(dasein->script.code.back());
	command_success_nodata(si, "[%d]@%-3zu@ %s",
	                       dasein->pid,
	                       dasein->script.lines(),
	                       string(local(line)));
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_pop =
{
    "U",
    N_(N_("Erase (pop) the last line of code (foreground).")),
    AC_AUTHENTICATED,
    0,
    ecma_pop,
    { NULL, NULL }
};


void ecma_stop(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
		return;

	if(!ecma->signal(si, *dasein, SIGSTOP))
		command_success_nodata(si, "Failed to SIGSTOP %d", dasein->pid);

	command_success_nodata(si, "[%d]+ @%zu Stopped",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_stop =
{
    "Z",
    N_(N_("SIGSTOP the foreground process.")),
    AC_AUTHENTICATED,
    0,
    ecma_stop,
    { NULL, NULL }
};


void ecma_quit(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
		return;

	if(!ecma->signal(si, *dasein, SIGQUIT))
		command_success_nodata(si, "Failed to SIGQUIT %d", dasein->pid);

	command_success_nodata(si, "[%d]+ @%zu Quit",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_quit =
{
    "\\",
    N_(N_("SIGQUIT the foreground process.")),
    AC_AUTHENTICATED,
    0,
    ecma_quit,
    { NULL, NULL }
};


void ecma_intr(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
		return;

	if(!ecma->signal(si, *dasein, SIGINT))
		command_success_nodata(si, "Failed to SIGINT %d", dasein->pid);

	command_success_nodata(si, "[%d]+ @%zu Interrupted",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_intr =
{
    "C",
    N_(N_("SIGINT the foreground process.")),
    AC_AUTHENTICATED,
    0,
    ecma_intr,
    { NULL, NULL }
};


void ecma_cont(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	const auto dasein(ecma->vorhanden.foreground(si->su));
	if(!dasein)
		return;

	if(!ecma->signal(si, *dasein, SIGCONT))
		command_success_nodata(si, "Failed to SIGCONT %d", dasein->pid);

	command_success_nodata(si, "[%d] @%zu",
	                       dasein->pid,
	                       dasein->script.lines());
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_cont =
{
    "CONT",
    N_(N_("SIGCONT the foreground process.")),
    AC_AUTHENTICATED,
    0,
    ecma_cont,
    { NULL, NULL }
};


void ecma_kill(sourceinfo_t *const si, int parc, char **parv)
noexcept try
{
	if(parc < 1)
	{
		const auto dasein(ecma->vorhanden.foreground(si->su));
		if(!dasein)
		{
			command_fail(si, fault_needmoreparams, "Usage: KILL [-signum] <pid>");
			return;
		}

		if(!ecma->signal(si, *dasein, SIGTERM))
			command_success_nodata(si, "Failed to SIGTERM %d", dasein->pid);

		return;
	}

	uint signum(SIGTERM);
	if(parc > 1)
		signum = abs(atol(parv[0]));

	if(!signum || signum >= 32)
	{
		command_fail(si, fault_badparams, "Bad signal number");
		return;
	}

	const auto pid(parc > 1? atol(parv[1]) : atol(parv[0]));
	if(!ecma->signal(si, pid, signum))
	{
		command_success_nodata(si, "Failed to send signal %u to %ld", signum, pid);
		return;
	}
}
catch(const std::exception &e)
{
	command_success_nodata(si, "Error: %s", e.what());
}
catch(...)
{
	slog(LG_ERROR, "ECMA: Unhandled exception of unknown type.");
}

command_t cmd_kill =
{
    "KILL",
    N_(N_("Send a signal to a process; or SIGTERM the foreground.")),
    AC_AUTHENTICATED,
    2,
    ecma_kill,
    { NULL, NULL }
};
*/
