/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
void path(const char *const &keys,
          const std::function<void (v8::Local<v8::String>)> &closure)
{
	tokens(keys, ".", [&closure]
	(char *const &key)
	{
		closure(string(key));
	});
}
