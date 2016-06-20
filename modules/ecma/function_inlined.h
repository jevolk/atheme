/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct inlined
:factory
{
	using options = factory::options;
	using call_ret = factory::call_ret;
	using call_arg = factory::call_arg;
	using function = std::function<call_ret (const call_arg &)>;

	function f;

	call_ret call(const call_arg &) override;

	inlined(const options & = {"<unnamed>", 0, true, false, false, false},
	        const function & = {});
};


inline
inlined::inlined(const options &o,
	             const function &f)
:factory{o}
,f{f}
{
}


inline
inlined::call_ret
inlined::call(const call_arg &arg)
{
	return f? f(arg) : call_ret{};
}
