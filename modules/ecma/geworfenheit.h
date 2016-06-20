/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct geworfenheit
:v8::Global<v8::Function>
{
	bool operator!() const                       { return !test(*this);                            }
	operator bool() const                        { return test(*this);                             }
	operator v8::Local<v8::Function>()           { return local(*this);                            }

	void clear()                                 { Reset();                                        }

	geworfenheit(v8::Local<v8::Function> = {});
	geworfenheit &operator=(v8::Local<v8::Function>) &;
};


inline
geworfenheit::geworfenheit(v8::Local<v8::Function> function)
:v8::Global<v8::Function>{isolate(), function}
{
}


inline
geworfenheit &geworfenheit::operator=(v8::Local<v8::Function> function)
&
{
	Reset(isolate(), function);
	return *this;
}
