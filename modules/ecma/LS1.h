/*
 * Copyright (C) 2016 Jason Volk
 *
 * The replacement for displacement is technology.

                                                         _.-'"""`-.
                                        |\         _.--'    _.._  `.
                    .-'""`-._           \ (  __.--'    _.-'"    \   \_._
                 .-'.-'_.---.`-.      .-_)\\\ \   _.-''       __.\  _L  `.
                / .'.'       `-.`._.-' \ \|||_|--'|          /  `-\' \    \
               J J / '       .-'`.    __|--'|_|--Y       _.-'    \_\-')    \
               F           .-')|_|---'        |  |   _.-'        __ ||      L
              J     |    .'_.-'/' ||--.___    | _|--'          /| .' <      |
              J |        `-'      |<_.    `--.-'              ( \ (_' L     J
               L     \            F | `--.-'                   \_`-'  |     |
               A  \              /  F .-'                        "    J     F
               |\  `.   ` .  _  '   .'                                |  _.'
               | \    .       .'   /                                  F-'
               |\ `.      -      .'                              .-'-'
               J L  `.         .'/                            .-'
                FJ     `--..--' /                    _.----'-'.\
                | \     / / /  /                 _.-'.'_.--._  `\
               .'|\\   / / /  J                 .'  /.'      `. \\
              <_.->\\ / /\/   F                //  //--.       \ \\
           _.-'-_)  \\ /     J                J/  JJ |  `.      \ \`-.
        -'"--'       \\      |                F   || |   )\      L L |
        \'   \  .---./\\     |               J|   || .-./  L     | |<
         \    L/ .-./  \\    |               ||   JJ((_))--|     | J|L
          L   J  (_)    \\  .|               |J    \\`-'   F     J |||
          J    \   |     L '      _.-'>      J \    \\`.   F     / F |
           \   _>-'      J     _.-'<-'        \ \    `. `./    .'./  |
          _>-'     |      \   J`.   \  _.      \ `.   `..   _.' .'   F
         <        \|       \  | O|   |'-'       `. `.   `-._.--'    J
          L        |        \ \  |.-'             `-._  _.--'       |)
          J        J         `'\-'                    ""            /
           \     .-'            \                                  /
            ) --'   L            \                                /
           //\      \             `.                            .'
         .''.-.      \              `.                        .'
         || (_)                       `-.                   .'
          `-.  \       \                 `--.____     ___.-/
             `. \                          \ \   \"""_.'  /
               \ \      _        _.______.__`.`.--`-'   .'\
                `-.__.-' \      /\ \     \           .<'   \
                          L     || |   __||_______.-' \  .-'>
                          )     `--'"""  '  _/         <-'  \
                        .'   _.-'     __.-'"            \    \
                      .' .-'     _.--'                   \    \     VK
                      \'    _.-'                          \ .'|
                       \_.-'                              <_.-' 


 * but a cookie-cutter LS1 swap will have to do.
 */


namespace LS1 {
using namespace v8;


struct string
:Local<String>
{
	string(v8::MaybeLocal<v8::Value> val);
	string(v8::Isolate *const &e, const char *const str, const String::NewStringType & = String::kNormalString);
	string(const char *const &str = "", const String::NewStringType & = String::kNormalString);
	explicit string(const std::string &str, const String::NewStringType & = String::kNormalString);
};


inline
string::string(const std::string &str,
               const String::NewStringType &type)
:string(str.c_str(), type)
{
}


inline
string::string(const char *const &str,
               const String::NewStringType &type)
:string(isolate(), str, type)
{
}


inline
string::string(v8::MaybeLocal<v8::Value> ml)
:Local<String>{[&ml]
{
	auto ret(maybe(ml));
	if(!ret->IsString())
		throw type_error("not a string");

	return ret.As<String>();
}()}
{
}


inline
string::string(v8::Isolate *const &iso,
               const char *const str,
               const String::NewStringType &type)
:Local<String>{str? String::NewFromUtf8(iso, str, type) : String::Empty(iso) }
{
}



struct string_object
:Local<StringObject>
{
	template<class... args> string_object(args&&... a);
};


template<class... args>
string_object::string_object(args&&... a)
:Local<StringObject>
{
	StringObject::New(string(std::forward<args>(a)...)).As<StringObject>()
}
{
}



struct integer
:Local<Integer>
{
	explicit operator int64_t() const
	{
		return (*this)->Value();
	}

	integer(v8::MaybeLocal<v8::Value> val);
	integer(v8::Isolate *const &e, const long &value);
	integer(const long &value = 0);
};


inline
integer::integer(const long &value)
:integer(isolate(), value)
{
}


inline
integer::integer(v8::Isolate *const &iso,
                 const long &value)
:Local<Integer>{Integer::New(iso, value)}
{
}


inline
integer::integer(v8::MaybeLocal<v8::Value> ml)
:Local<Integer>{[&ml]
{
	auto ret(maybe(ml));
	if(!ret->IsInt32() && !ret->IsUint32())
		throw type_error("not an integer");

	return ret.As<Integer>();
}()}
{
}



struct boolean
:Local<Boolean>
{
	boolean(v8::MaybeLocal<v8::Value> val);
	boolean(v8::Isolate *const &e, const bool &val);
	boolean(const bool &val = false);
};


inline
boolean::boolean(const bool &value)
:boolean(isolate(), value)
{
}


inline
boolean::boolean(v8::Isolate *const &iso,
                 const bool &value)
:Local<Boolean>{Boolean::New(iso, value)}
{
}


inline
boolean::boolean(v8::MaybeLocal<v8::Value> ml)
:Local<Boolean>{[&ml]
{
	auto ret(maybe(ml));
	if(!ret->IsBoolean())
		throw type_error("not a boolean");

	return ret.As<Boolean>();
}()}
{
}



struct array
:Local<Array>
{
	struct iterator
	{
		array *a;
		size_t i;
	};

	v8::Local<v8::Value> operator[](const size_t &i);
	size_t size() const;

	template<class T> array(std::initializer_list<v8::Local<T>> list);
	array(v8::Isolate *const &iso, const size_t &size);
	array(const size_t &size = 0);
};


template<class T>
array::array(std::initializer_list<v8::Local<T>> list)
:array{list.size()}
{
	size_t i(0);
	std::for_each(begin(list), end(list), [this, &i]
	(auto &elem)
	{
		set(*this, i++, elem);
	});
}


inline
array::array(v8::Isolate *const &iso,
             const size_t &size)
:Local<Array>{Array::New(iso, size)}
{
}


inline
array::array(const size_t &size)
:array{isolate(), size}
{
}


inline
size_t array::size()
const
{
	return (*this)->Length();
}


inline
v8::Local<v8::Value>
array::operator[](const size_t &i)
{
	using namespace v8;

	auto &super(static_cast<Local<Array> &>(*this));
	return get(super, i);
}


inline
array::iterator begin(array &a)
{
	return array::iterator { &a, 0 };
}


inline
array::iterator end(array &a)
{
	return array::iterator { &a, a->Length() };
}


inline
v8::Local<v8::Value>
operator*(array::iterator &a)
{
	using namespace v8;

	auto &super(static_cast<Local<Array> &>(*a.a));
	return get(super, a.i);
}


inline
array::iterator &operator++(array::iterator &a)
{
	++a.i;
	return a;
}


inline
array::iterator &operator--(array::iterator &a)
{
	--a.i;
	return a;
}


inline
bool operator==(const array::iterator &a, const array::iterator &b)
{
	return a.i == b.i;
}


inline
bool operator!=(const array::iterator &a, const array::iterator &b)
{
	return a.i != b.i;
}


inline
bool operator<(const array::iterator &a, const array::iterator &b)
{
	return a.i < b.i;
}



struct map
:Local<Map>
{
	using Local<Map>::operator->;

	template<class engine> map(engine &);
};


template<class engine>
map::map(engine &e)
:Local<Map>{Map::New(e)}
{
}



struct function
:Local<Function>
{
	template<class context, class call, class data> function(context &, const call &, data&&);
};


template<class context,
         class call,
         class data>
function::function(context &c,
                   const call &cb,
                   data&& d)
:Local<Function>{maybe(Function::New(c, cb, d))}
{
}



struct external
:Local<External>
{
	external(void *const &ptr = nullptr);
};


inline
external::external(void *const &ptr)
:Local<External>{External::New(isolate(), ptr)}
{
}


} // namespace LS1
