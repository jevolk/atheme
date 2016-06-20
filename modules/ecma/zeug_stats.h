/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

size_t heap_stats_string(char *const &buf, const size_t &max, v8::HeapObjectStatistics &hs);
size_t heap_stats_string(char *const &buf, const size_t &max, v8::HeapSpaceStatistics &hs);
size_t heap_stats_string(char *const &buf, const size_t &max, v8::HeapStatistics &hs);

struct stats
{
	struct counters counters;                    // The engine will report here.
	uint64_t time              { 0          };   // Microseconds spent in engine execution
	uint64_t runs              { 0          };   // Number of engine entrances
};


inline
size_t heap_stats_string(char *const &buf,
                         const size_t &max,
                         v8::HeapStatistics &hs)
{
	return snprintf(buf, max,
	                "size: %zu exec: %zu phys: %zu avail: %zu used: %zu limit: %zu malloc: %zu garbage: %zu",
	                hs.total_heap_size(),
	                hs.total_heap_size_executable(),
	                hs.total_physical_size(),
	                hs.total_available_size(),
	                hs.used_heap_size(),
	                hs.heap_size_limit(),
	                hs.malloced_memory(),
	                hs.does_zap_garbage());
}


inline
size_t heap_stats_string(char *const &buf,
                         const size_t &max,
                         v8::HeapSpaceStatistics &hs)
{
	return snprintf(buf, max,
	                "name: %s size: %zu used: %zu avail: %zu phys: %zu",
	                hs.space_name(),
	                hs.space_size(),
	                hs.space_used_size(),
	                hs.space_available_size(),
	                hs.physical_space_size());
}


inline
size_t heap_stats_string(char *const &buf,
                         const size_t &max,
                         v8::HeapObjectStatistics &hs)
{
	return snprintf(buf, max,
	                "type: %s subtype: %s count: %zu size: %zu",
	                hs.object_type(),
	                hs.object_sub_type(),
	                hs.object_count(),
	                hs.object_size());
}
