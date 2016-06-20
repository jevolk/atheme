/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct policy
{
	uint64_t cpu_interval     = 50000;
	uint64_t cpu_quota        = 1000 * 1000 * 5;
	uint64_t cpu_runs         = -1;
	int8_t cpu_nice           = 0;
}

static policy_default;
