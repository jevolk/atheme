/**
 *	COPYRIGHT 2014 (C) Jason Volk
 *	COPYRIGHT 2014 (C) Svetlana Tkachenko
 *
 *	DISTRIBUTED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) (see: LICENSE)
 */


enum color_mode
{
	COLOR_OFF        = 0x0f,
	COLOR_BOLD       = 0x02,
	COLOR_ON         = 0x03,
	COLOR_ITALIC     = 0x09,
	COLOR_STRIKE     = 0x13,
	COLOR_UNDER      = 0x15,
	COLOR_UNDER2     = 0x1f,
	COLOR_REVERSE    = 0x16,
};

enum color_fg
{
	FG_WHITE,         FG_BLACK,      FG_BLUE,      FG_GREEN,
	FG_LRED,          FG_RED,        FG_MAGENTA,   FG_ORANGE,
	FG_COLOR_YELLOW,  FG_LGREEN,     FG_CYAN,      FG_LCYAN,
	FG_LBLUE,         FG_LMAGENTA,   FG_GRAY,      FG_LGRAY
};

enum color_bg
{
	BG_LGRAY_BLINK,     BG_BLACK,           BG_BLUE,          BG_GREEN,
	BG_RED_BLINK,       BG_RED,             BG_MAGENTA,       BG_ORANGE,
	BG_ORANGE_BLINK,    BG_GREEN_BLINK,     BG_CYAN,          BG_CYAN_BLINK,
	BG_BLUE_BLINK,      BG_MAGENTA_BLINK,   BG_BLACK_BLINK,   BG_LGRAY,
};
