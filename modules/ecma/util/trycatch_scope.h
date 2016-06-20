/*
 * Copyright (C) 2016 Jason Volk
 *
 */

struct trycatch_scope
{
	v8::TryCatch tc;

	operator const v8::TryCatch &() const        { return tc;                                      }
	operator v8::TryCatch &()                    { return tc;                                      }
	auto operator->() const                      { return &tc;                                     }
	auto operator->()                            { return &tc;                                     }

	trycatch_scope();
};


inline
trycatch_scope::trycatch_scope()
:tc(v8::Isolate::GetCurrent())
{
}
