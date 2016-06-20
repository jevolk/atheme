/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

struct exception
:std::exception
{
	using ctor = v8::Local<v8::Value> (&)(v8::Local<v8::String>);

	v8::Global<v8::Value> value;

	const char *what() const noexcept override   { return string(local(value));                    }
	operator v8::Local<v8::Value>()              { return local(value);                            }

	exception(v8::Local<v8::Message>) noexcept;
	exception(v8::Local<v8::Value>) noexcept;

	exception(const ctor &             = v8::Exception::Error,
	          const char *const &name  = "Unknown",
	          const char *const fmt    = " ",
	          ...) noexcept PRINTFLIKE(4, 5);
};


/*
 * Fundamental v8 js exceptions. These can be thrown for js/v8 related errors
 * if the POSIX system enumeration (below) isn't appropriate.
 */

// The name of the exception is given only by the most-derived class. pass_name_t
// disambiguates constructors to allow intermediate classes to pass the name along.
ECMA_OVERLOAD(pass_name)

#define ECMA_EXCEPTION_V8(name, ctor)                                 \
struct name                                                           \
:exception                                                            \
{                                                                     \
    template<class... A>                                              \
    name(const pass_name_t &, A&&... a):                              \
    exception { ctor, std::forward<A>(a)... } {}                      \
                                                                      \
    template<class... A>                                              \
    name(A&&... a):                                                   \
    exception { ctor, "", std::forward<A>(a)... } {}                  \
                                                                      \
    name(v8::Local<v8::Value> value):      exception { value    } {}  \
    name(v8::Local<v8::Message> message):  exception { message  } {}  \
};

ECMA_EXCEPTION_V8( js_error,         v8::Exception::Error          )
ECMA_EXCEPTION_V8( type_error,       v8::Exception::TypeError      )
ECMA_EXCEPTION_V8( syntax_error,     v8::Exception::SyntaxError    )
ECMA_EXCEPTION_V8( reference_error,  v8::Exception::ReferenceError )
ECMA_EXCEPTION_V8( range_error,      v8::Exception::RangeError     )


/*
 * Our exceptions are consolidated as POSIX error codes.
 *
 * ex:            throw error<ENOENT>();
 * optionally:    throw error<ENOENT>("because %s does not exist", name);
 *
 * The template parameter is superior to a constructor argument because
 * it gives each exception strong typing for catch disambiguation (which we
 * have little use for since these mostly end up in js-land) and allows
 * the exception code to not clutter real argument composition passed up the
 * stack by cluttering the template values passed up the stack instead; creating
 * a solid hundred or so versions of each generated inline leaf jk :) maybe
 */

template<int code>
struct error
:js_error
{
	template<class... args> error(args&&...);
};


template<int code>
template<class... args>
error<code>::error(args&&... a)
:js_error
{
	pass_name,
	std::error_code(code, std::system_category()).message().c_str(),
	std::forward<args>(a)...
}
{
}


// Furthermore we can specialize some POSIX values to inherit from better fundamental
// js exception types.

#define ECMA_EXCEPTION_SPEC(js_error_type, posix_code)                          \
template<>                                                                      \
struct error<posix_code>                                                        \
:js_error_type                                                                  \
{                                                                               \
    template<class... args> error(args&&... a)                                  \
    :js_error_type                                                              \
    {                                                                           \
        pass_name,                                                              \
        std::error_code(posix_code, std::system_category()).message().c_str(),  \
        std::forward<args>(a)...                                                \
    } {}                                                                        \
};

ECMA_EXCEPTION_SPEC( type_error,       EINVAL           )
ECMA_EXCEPTION_SPEC( reference_error,  ENOENT           )



inline
exception::exception(const ctor &ctor,
                     const char *const &name,
                     const char *const fmt,
                     ...)
noexcept
{
	va_list ap;
	va_start(ap, fmt);

	size_t sz(0);
	char msg[BUFSIZE];
	const bool no_msg(!fmt || fmt[0] == '\0' || fmt[0] == ' ');
	const char *const sep(no_msg? "." : ": ");
	sz = mowgli_strlcpy(msg, name, sizeof(msg));
	sz = mowgli_strlcat(msg, sep, sizeof(msg));
	vsnprintf(msg + sz, sizeof(msg) - sz, fmt, ap);
	{
		const handle_scope handle_scope;
		this->value.Reset(isolate(), ctor(string(msg)));
	}

	va_end(ap);
}


inline
exception::exception(v8::Local<v8::Value> value)
noexcept
:value{isolate(), value}
{
}


inline
exception::exception(v8::Local<v8::Message> message)
noexcept
:value{isolate(), test(message)? message->Get() : v8::Local<v8::String>{}}
{
}
