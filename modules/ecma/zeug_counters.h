/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct counter
{
	const char *name { nullptr };
	int value { 0 };
};

struct counters
{
	static constexpr size_t MAX { 1024 };

	std::array<counter, MAX> counters;
	size_t used { 0 };

	const auto &operator[](const size_t &pos) const;
	auto &operator[](const size_t &pos);

	size_t next(const char *const name);
	int *lookup(const char *const name);
};


inline
int *counters::lookup(const char *const name)
{
	const auto pos(next(name));
	return pos < MAX? &counters[pos].value : nullptr;
}


inline
size_t counters::next(const char *const name)
{
	if(used >= MAX)
		return MAX;

	const auto b(begin(counters));
	const auto e(std::next(begin(counters), used));
	const auto it(std::find_if(b, e, [&name]
	(const counter &counter)
	{
		return strcmp(name, counter.name) == 0;
	}));

	auto &counter(*it);
	counter.name = name;
	return used++;
}


inline
auto &counters::operator[](const size_t &pos)
{
	return counters[pos];
}


inline
const auto &counters::operator[](const size_t &pos)
const
{
	return counters[pos];
}
