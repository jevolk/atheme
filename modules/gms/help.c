/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "gms.h"


const char *const logo[] =
{
"                                                               ",
"   .-_'''-.  ,---.    ,---.  .-'''-.                           ",
"  '_( )_   \\ |    \\  /    | / _     \\                       ",
" |(_ o _)|  '|  ,  \\/  ,  |(`' )/`--'                         ",
" . (_,_)/___||  |\\_   /|  (_ o _).                            ",
" |  |  .-----|  _( )_/ |  |(_,_). '.                           ",
" '  \\  '-   .| (_ o _) |  .---.  \\  :                        ",
"  \\  `-'`   ||  (_,_)  |  \\    `-'  |                        ",
"   \\        /|  |      |  |\\       /                         ",
"    `'-...-' '--'      '--' `-...-'                            ",
"                                                               ",
};

short cmap[12][64];
char clogo[12][256];

static
void cmap_init(void)
{
	for(size_t i = 0; i < 10; i++)
		for(size_t j = 0; j < 63; j++)
			if(logo[i][j] == ')' || logo[i][j] == '(')
			{
				if(j < 25)
					cmap[i][j] = FG_COLOR_YELLOW;
				else
					cmap[i][j] = FG_LMAGENTA;
			}
			else if(logo[i][j] != ' ')
				cmap[i][j] = FG_GRAY;

	cmap[2][3] = FG_COLOR_YELLOW;
	cmap[1][5] = FG_COLOR_YELLOW;
	cmap[2][7] = FG_COLOR_YELLOW;
	cmap[3][7] = FG_COLOR_YELLOW;
	cmap[3][3] = FG_COLOR_YELLOW;
	cmap[4][4] = FG_COLOR_YELLOW;
	cmap[4][6] = FG_COLOR_YELLOW;
	cmap[3][5] = FG_LGREEN;
	cmap[4][5] = FG_LGREEN;

	cmap[5][16] = FG_COLOR_YELLOW;
	cmap[6][16] = FG_COLOR_YELLOW;
	cmap[7][17] = FG_COLOR_YELLOW;
	cmap[7][18] = FG_LGREEN;
	cmap[7][19] = FG_COLOR_YELLOW;
	cmap[6][20] = FG_COLOR_YELLOW;
	cmap[5][20] = FG_COLOR_YELLOW;
	cmap[4][18] = FG_COLOR_YELLOW;
	cmap[6][18] = FG_LGREEN;

	cmap[4][27] = FG_LMAGENTA;
	cmap[5][28] = FG_LMAGENTA;
	cmap[5][29] = FG_LGREEN;
	cmap[5][30] = FG_LMAGENTA;
	cmap[4][31] = FG_LMAGENTA;
	cmap[2][30] = FG_LMAGENTA;
	cmap[3][29] = FG_LMAGENTA;
	cmap[3][28] = FG_LMAGENTA;
	cmap[4][29] = FG_LGREEN;

	for(size_t i = 0; i < 10; i++)
		for(size_t j = 0, cj = 0; j < 63; j++)
			if(cmap[i][j])
				cj += sprintf(clogo[i] + cj, "\x03%02d%c\x0f", cmap[i][j], logo[i][j]);
			else
				clogo[i][cj++] = logo[i][j];
}


void gms_help(sourceinfo_t *si, int parc, char *parv[])
{
	//spqf->helpcmd.first = parc;
	//spqf->helpcmd.second = parv;

	if(!parc)
	{
		command_success_nodata(si, _("****** \2" GMS_TITLE "\2 ******"));

		//for(size_t i = 0; i < sizeof(logo) / sizeof(char*); i++)
		//	command_success_nodata(si, _("%s"), logo[i]);

		for(size_t i = 0; i < 12; i++)
			command_success_nodata(si, "%s", clogo[i]);

		command_success_nodata(si, _("\2%s is your service to organize projects and collaborate on freenode\2."), si->service->nick);
		command_success_nodata(si, _("[%c%02dBETA!%c] Visit us @ \2#freenode-gms\2 for questions & comments."), COLOR_ON, FG_LBLUE, COLOR_OFF);
		command_success_nodata(si, _(""));
		command_success_nodata(si, _("For more information on a command, type:"));
		command_success_nodata(si, "\2/%s%s help <command>\2", (ircd->uses_rcommand == false) ? "msg " : "", si->service->nick);
		command_success_nodata(si, " ");

		command_help(si, si->service->commands);

		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}
	else help_display_as_subcmd(si, si->service, NULL, parv[0], si->service->commands);
}


command_t cmd =
{
	"HELP",
	N_(N_("Displays contextual help information.")),
	AC_NONE,
	6,
	gms_help,
	{ NULL, NULL }
};


void module_init(module_t *const m)
{
	module_init_common(m);
	cmap_init();
	service_bind_command(myservice, &cmd);
}


void module_fini(module_unload_intent_t intent)
{
	service_unbind_command(myservice, &cmd);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	GMS_MODULE"/help",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
