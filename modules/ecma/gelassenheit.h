/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct gelassenheit
:v8::Global<v8::Object>
{
	static void assert_generator(v8::Local<v8::Object>);

	bool operator!() const                       { return !test(*this);                            }
	operator bool() const                        { return test(*this);                             }
	operator v8::Local<v8::Value>()              { return local(*this);                            }

	auto get(const char *const &key)             { return ::get<v8::Function>(local(*this), key);  }
	auto get_return()                            { return get("return");                           }
	auto get_throw()                             { return get("throw");                            }
	auto get_next()                              { return get("next");                             }
	void clear()                                 { Reset();                                        }

	gelassenheit(v8::Local<v8::Object> = {});
	gelassenheit &operator=(v8::Local<v8::Object>) &;
};


inline
gelassenheit::gelassenheit(v8::Local<v8::Object> generator)
:v8::Global<v8::Object>{isolate(), generator}
{
	assert_generator(generator);
}


inline
gelassenheit &gelassenheit::operator=(v8::Local<v8::Object> generator)
&
{
	assert_generator(generator);
	Reset(isolate(), generator);
	return *this;
}


inline
void gelassenheit::assert_generator(v8::Local<v8::Object> object)
{
	if(unlikely(test(object) && !object->IsGeneratorObject()))
		throw type_error("Not a generator object");
}
