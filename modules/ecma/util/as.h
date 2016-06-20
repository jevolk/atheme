/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T> constexpr bool is_v8();
template<class R> constexpr bool is_boolean();
template<class R> constexpr bool is_floating();
template<class R> constexpr bool is_signed_int();
template<class R> constexpr bool is_unsigned_int();
template<class R> constexpr bool is_cstring();
template<class R> constexpr bool is_extptr();
template<class R> constexpr bool is_extref();
template<template<class> class Local, class T> constexpr bool is_local();

template<class R> using returns_local         = typename std::enable_if<is_v8<R>(), v8::Local<R>>::type;
template<class R> using returns_bool          = typename std::enable_if<is_boolean<R>(), R>::type;
template<class R> using returns_float         = typename std::enable_if<is_floating<R>(), R>::type;
template<class R> using returns_signed_int    = typename std::enable_if<is_signed_int<R>(), R>::type;
template<class R> using returns_unsigned_int  = typename std::enable_if<is_unsigned_int<R>(), R>::type;
template<class R> using returns_cstring       = typename std::enable_if<is_cstring<R>(), R>::type;
template<class R> using returns_pointer       = typename std::enable_if<is_extptr<R>(), R>::type;
template<class R> using returns_reference     = typename std::enable_if<is_extref<R>(), R>::type;


template<bool (v8::Value::* functor)() const>
struct value_is
{
	bool ret;

	operator bool() const                        { return ret;                                     }
	bool operator!() const                       { return !ret;                                    }

	value_is(const v8::Local<v8::Value> &val)
	:ret                                         { std::bind(functor, *val)()                      }
	                                             {                                                 }
};


template<class T>
struct is
{
	static constexpr const char *const &name     { "Unknown"                                       };

	operator bool() const                        { throw type_error("is unknown type");            }
	bool operator !() const                      { throw type_error("is unknown type");            }

	is(const v8::Local<T> &val)                  {                                                 }
};


template<>
struct is<v8::Value>
{
	static constexpr const char *const &name     { "Value"                                         };

	operator bool() const                        { return true;                                    }
	bool operator !() const                      { return false;                                   }

	is(const v8::Local<v8::Value> &val)          {                                                 }
};


#define ECMA_DEFINE_VALUE_IS(type, reflection, functor)       \
template<>                                                    \
struct is<type>                                               \
:value_is<&functor>                                           \
{                                                             \
    static constexpr const char *const &name { reflection };  \
    using value_is::value_is;                                 \
};

ECMA_DEFINE_VALUE_IS(v8::Name,         "Name",       v8::Value::IsName)
ECMA_DEFINE_VALUE_IS(v8::String,       "String",     v8::Value::IsString)
ECMA_DEFINE_VALUE_IS(v8::Symbol,       "Symbol",     v8::Value::IsSymbol)
ECMA_DEFINE_VALUE_IS(v8::Function,     "Function",   v8::Value::IsFunction)
ECMA_DEFINE_VALUE_IS(v8::Array,        "Array",      v8::Value::IsArray)
ECMA_DEFINE_VALUE_IS(v8::Object,       "Object",     v8::Value::IsObject)
ECMA_DEFINE_VALUE_IS(v8::Boolean,      "Boolean",    v8::Value::IsBoolean)
ECMA_DEFINE_VALUE_IS(v8::Number,       "Number",     v8::Value::IsNumber)
ECMA_DEFINE_VALUE_IS(v8::Uint32,       "Uint32",     v8::Value::IsUint32)
ECMA_DEFINE_VALUE_IS(v8::Int32,        "Int32",      v8::Value::IsInt32)
ECMA_DEFINE_VALUE_IS(v8::External,     "External",   v8::Value::IsExternal)


template<class T>
auto mustbe(v8::Local<v8::Value> val,
            const char *const errmsg = nullptr)
{
	if(likely(is<T>(val)))
		return val.As<T>();

	if(errmsg)
		throw type_error(errmsg);
	else
		throw type_error("Type must be: %s", is<T>::name);
}


template<class R>
returns_local<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	return mustbe<R>(val, errmsg);
}


template<class R>
returns_bool<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::Boolean>(val, errmsg));
	return ret->Value();
}


template<class R>
returns_float<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::Number>(val, errmsg));
	return ret->Value();
}


template<class R>
returns_unsigned_int<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::Uint32>(val, errmsg));
	return ret->Value();
}


template<class R>
returns_signed_int<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::Int32>(val, errmsg));
	return ret->Value();
}


template<class R>
returns_cstring<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::String>(val, errmsg));
	return string(ret);
}


template<class R>
returns_pointer<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	auto ret(mustbe<v8::External>(val, errmsg));
	return static_cast<R>(ret->Value());
}


template<class R>
returns_reference<R>
as(const v8::Local<v8::Value> &val,
   const char *const errmsg = nullptr)
{
	using value_type = typename std::remove_reference<R>::type;
	using pointer_type = typename std::add_pointer<value_type>::type;

	auto ret(mustbe<v8::External>(val, errmsg));
	return *static_cast<pointer_type>(ret->Value());
}


template<template<class>
         class Local,
         class T>
constexpr
bool is_local()
{
	return is_v8<T>() && std::is_same<Local<T>, v8::Local<T>>::value;
}


template<class R>
constexpr
bool is_extref()
{
	return !is_v8<R>() && std::is_lvalue_reference<R>::value;
}


template<class R>
constexpr
bool is_extptr()
{
	return !is_v8<R>() && !is_cstring<R>() && std::is_pointer<R>::value;
}


template<class R>
constexpr
bool is_cstring()
{
	return !is_v8<R>() && std::is_same<R, const char *>::value;
}


template<class R>
constexpr
bool is_unsigned_int()
{
	return !is_v8<R>() && !is_boolean<R>() && std::is_integral<R>::value && !std::is_signed<R>();
}


template<class R>
constexpr
bool is_signed_int()
{
	return !is_v8<R>() && !is_boolean<R>() && std::is_integral<R>::value && std::is_signed<R>::value;
}


template<class R>
constexpr
bool is_floating()
{
	return !is_v8<R>() && std::is_floating_point<R>::value;
}


template<class R>
constexpr
bool is_boolean()
{
	return !is_v8<R>() && std::is_same<R, bool>::value;
}


template<class T>
constexpr
bool is_v8()
{
	return std::is_base_of<v8::Data, T>::value;
}
