/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


#define likely(pred) __builtin_expect((pred),1)
#define unlikely(pred) __builtin_expect((pred),0)

#define ECMA_EXPCAT(a, b)   a ## b
#define ECMA_CONCAT(a, b)   ECMA_EXPCAT(a, b)
#define ECMA_UNIQUE(a)      ECMA_CONCAT(a, __COUNTER__)


#define ECMA_OVERLOAD(NAME)             \
    struct NAME##_t {};                 \
    static constexpr NAME##_t NAME {};


#define ECMA_STRONG_TYPEDEF(TYPE, NAME)                                  \
struct NAME                                                              \
{                                                                        \
    TYPE val;                                                            \
                                                                         \
    operator const TYPE &() const   { return val;  }                     \
    operator TYPE &()               { return val;  }                     \
};


// ex: using foo_t = ECMA_STRONG_T(int)
#define ECMA_STRONG_T(TYPE) \
    ECMA_STRONG_TYPEDEF(TYPE, ECMA_UNIQUE(strong_t))


struct scope
{
	const std::function<void ()> func;

	template<class Func> scope(Func&& func): func(std::forward<Func>(func)) {}
	~scope() { func(); }
};


template<int (&ctype)(int)>
bool all(const char *const &val)
{
	return std::all_of(val, val + strlen(val), ctype);
}


template<int (&ctype)(int)>
bool any(const char *const &val)
{
	return std::any_of(val, val + strlen(val), ctype);
}


template<int (&ctype)(int)>
bool none(const char *const &val)
{
	return std::none_of(val, val + strlen(val), ctype);
}


constexpr
size_t hash(const char *const &str,
            const size_t i = 0)
{
	return !str[i]? 7681ULL : (hash(str, i+1) * 33ULL) ^ str[i];
}


inline
size_t hash(const std::string &str,
            const size_t i = 0)
{
	return i >= str.size()? 7681ULL : (hash(str, i+1) * 33ULL) ^ str.at(i);
}


inline
void tokens(char *const &str,
            const char *const &delim,
            const std::function<void (char *)> &lambda)
{
	char *ctx;
	char *tok(strtok_r(str, delim, &ctx)); do
	{
		lambda(tok);
	}
	while((tok = strtok_r(NULL, delim, &ctx)) != NULL);
}


inline
void tokens(const char *const &str,
            const char *const &delim,
            const std::function<void (char *)> &lambda)
{
	char buf[BUFSIZE];
	mowgli_strlcpy(buf, str, sizeof(buf));
	tokens(buf, delim, lambda);
}


inline
void tokensa(const char *const &str,
             const char *const &delim,
             const std::function<void (char *)> &lambda)
{
	const auto free([](void *const ptr) { mowgli_free(ptr); });
	const std::unique_ptr<char[], void(*)(void *)> cpy(mowgli_strdup(str), free);
	tokens(cpy.get(), delim, lambda);
}


inline
size_t tokens_count(const char *const &str,
                    const char *const &delim)
{
	size_t ret(0);
	tokens(str, delim, [&ret]
	(char *const token)
	{
		++ret;
	});

	return ret;
}


inline
std::pair<std::string, std::string>
split(const std::string &str,
      const std::string &delim = " ")
{
	const auto pos(str.find(delim));
	return pos == std::string::npos?
	              std::make_pair(str, std::string{}):
	              std::make_pair(str.substr(0,pos), str.substr(pos + delim.size()));
}


// For conforming enums
template<class Enum>
constexpr
typename std::underlying_type<Enum>::type
num_of()
{
	return static_cast<typename std::underlying_type<Enum>::type>(Enum::_NUM_);
}


template<class Enum>
uint index(const char *const &str,
           const size_t &n = 16)
{
	size_t i(0);
	for(; i < num_of<Enum>(); ++i)
		if(strncasecmp(str, reflect(static_cast<Enum>(i)), n) == 0)
			break;

	return i;
}


template<class Enum>
bool exists(const char *const &str,
            const size_t &n = 16)
{
	return index<Enum>(str, n) < num_of<Enum>();
}


template<class Enum>
Enum reflect(const char *const &str,
             const size_t &n = 16)
{
	const auto idx(index<Enum>(str, n));
	if(idx >= num_of<Enum>())
		throw std::out_of_range("string is not reflected by the enum");

	return static_cast<Enum>(idx);
}


template<class Enum>
void for_each(const std::function<void (const Enum &)> &func)
{
	for(size_t i(0); i < num_of<Enum>(); ++i)
		func(static_cast<Enum>(i));
}


inline
void channel_mode(user_t *const &src,
                  channel_t *const &targ,
                  const std::string &effect)
{
	char buf[effect.size() + 1];
	mowgli_strlcpy(buf, effect.c_str(), sizeof(buf));

	std::vector<char *> parv;
	tokens(buf, " ", [&parv](char *const &arg)
	{
		parv.emplace_back(arg);
	});

	parv.emplace_back(nullptr);
	channel_mode(src, targ, parv.size(), parv.data());
}


static
void command_success_tty(sourceinfo_t *si, const size_t &linemax, const char *fmt, ...) PRINTFLIKE(3, 4);
void command_success_tty(sourceinfo_t *const si,
                         const size_t &linemax,
                         const char *const fmt,
                         ...)
{
	va_list ap;
	va_start(ap, fmt);
	static thread_local char buf[32768];
	const size_t len(vsnprintf(buf, sizeof(buf), fmt, ap));
	for(size_t i(0), e(0); i < len; i = e + 1)
	{
		e = std::min(i + linemax - 1, len - 1);
		while(e > i && !::isspace(buf[e])) --e;

		const char c(buf[e+1]);
		buf[e+1] = '\0';
		command_success_nodata(si, "%s", buf + i);
		buf[e+1] = c;
	}

	va_end(ap);
}


template<class function,
         class... args>
auto syscall(function&& f,
             args&&... a)
{
	const auto ret(f(a...));
	if(unlikely(long(ret) == -1))
	{
		char buf[128], ebuf[64];
		snprintf(buf, sizeof(buf), "system call error: %s",
		         strerror_r(errno, ebuf, sizeof(ebuf)));

		slog(LG_ERROR, "%s", buf);
		throw std::runtime_error(buf);
	}

	return ret;
}


struct critical_sigsection
{
	sigset_t set, old;

	critical_sigsection(const std::initializer_list<int> &sigs)
	{
		sigemptyset(&set);
		for(const auto &sig : sigs)
			sigaddset(&set, sig);

		syscall(::sigprocmask, SIG_BLOCK, &set, &old);
	}

	critical_sigsection()
	{
		sigfillset(&set);
		syscall(::sigprocmask, SIG_BLOCK, &set, &old);
	}

	~critical_sigsection()
	{
		syscall(::sigprocmask, SIG_SETMASK, &old, nullptr);
	}
};


inline
struct timeval usec_to_tv(const int64_t &us)
{
	struct timeval ret;
	ret.tv_sec = us / (1000L * 1000L);
	ret.tv_usec = us % (1000L * 1000L);
	return ret;
}


inline
uint64_t tv_to_usec(const struct timeval &tv)
{
	uint64_t ret(tv.tv_sec * 1000L * 1000L);
	ret += tv.tv_usec;
	return ret;
}
