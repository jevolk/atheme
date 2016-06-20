/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct opts
{
	sigset_t sigmask;
	bool event_logging                           { false                                           };

	opts();
};


inline
opts::opts()
:sigmask{[]
{
	sigset_t ret;
	sigemptyset(&ret);
	return ret;
}()}
{
}
