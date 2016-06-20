/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


inline
const char *reflect(const v8::Isolate::UseCounterFeature &f)
{
	using v8::Isolate;

	switch(f)
	{
		case Isolate::kUseAsm:                                     return "UseAsm";
		case Isolate::kBreakIterator:                              return "BreakIterator";
		case Isolate::kLegacyConst:                                return "LegacyConst";
		case Isolate::kMarkDequeOverflow:                          return "MarkDequeOverflow";
		case Isolate::kStoreBufferOverflow:                        return "StoreBufferOverflow";
		case Isolate::kSlotsBufferOverflow:                        return "SlotsBufferOverflow";
		case Isolate::kObjectObserve:                              return "ObjectObserve";
		case Isolate::kForcedGC:                                   return "ForcedGC";
		case Isolate::kSloppyMode:                                 return "SloppyMode";
		case Isolate::kStrictMode:                                 return "StrictMode";
		case Isolate::kStrongMode:                                 return "StrongMode";
		case Isolate::kRegExpPrototypeStickyGetter:                return "RegExpPrototypeStickyGetter";
		case Isolate::kRegExpPrototypeToString:                    return "RegExpPrototypeToString";
		case Isolate::kRegExpPrototypeUnicodeGetter:               return "RegExpPrototypeUnicodeGetter";
		case Isolate::kIntlV8Parse:                                return "IntlV8Parse";
		case Isolate::kIntlPattern:                                return "IntlPattern";
		case Isolate::kIntlResolved:                               return "IntlResolved";
		case Isolate::kPromiseChain:                               return "PromiseChain";
		case Isolate::kPromiseAccept:                              return "PromiseAccept";
		case Isolate::kPromiseDefer:                               return "PromiseDefer";
		case Isolate::kHtmlCommentInExternalScript:                return "HtmlCommentInExternalScript";
		case Isolate::kHtmlComment:                                return "HtmlComment";
		case Isolate::kSloppyModeBlockScopedFunctionRedefinition:  return "SloppyModeBlockScopedFunctionRedefinition";
		case Isolate::kForInInitializer:                           return "ForInInitializer";
		case Isolate::kArrayProtectorDirtied:                      return "ArrayProtectorDirtied";
		case Isolate::kArraySpeciesModified:                       return "ArraySpeciesModified";
		case Isolate::kArrayPrototypeConstructorModified:          return "ArrayPrototypeConstructorModified";
		case Isolate::kArrayInstanceProtoModified:                 return "ArrayInstanceProtoModified";
		case Isolate::kArrayInstanceConstructorModified:           return "ArrayInstanceConstructorModified";
		case Isolate::kLegacyFunctionDeclaration:                  return "LegacyFunctionDeclaration";
		case Isolate::kRegExpPrototypeSourceGetter:                return "RegExpPrototypeSourceGetter";
		case Isolate::kRegExpPrototypeOldFlagGetter:               return "RegExpPrototypeOldFlagGetter";
		case Isolate::kUseCounterFeatureCount:                     return "UseCounterFeatureCount";
		default:                                                   return "???????";
	}
}


inline
const char *reflect(const v8::JitCodeEvent::EventType &e)
{
	using namespace v8;

	switch(e)
	{
		case JitCodeEvent::CODE_ADDED:                      return "CODE_ADDED";
		case JitCodeEvent::CODE_MOVED:                      return "CODE_MOVED";
		case JitCodeEvent::CODE_REMOVED:                    return "CODE_REMOVED";
		case JitCodeEvent::CODE_ADD_LINE_POS_INFO:          return "CODE_ADD_LINE_POS_INFO";
		case JitCodeEvent::CODE_START_LINE_INFO_RECORDING:  return "CODE_START_LINE_INFO_RECORDING";
		case JitCodeEvent::CODE_END_LINE_INFO_RECORDING:    return "CODE_END_LINK_INFO_RECORDING";
		default:                                            return "??????";
	}
}


inline
int reflect_signal(const char *const &name)
{
	switch(hash(name))
	{
		case hash("SIGHUP"):     return SIGHUP;
		case hash("SIGINT"):     return SIGINT;
		case hash("SIGQUIT"):    return SIGQUIT;
		case hash("SIGILL"):     return SIGILL;
		case hash("SIGTRAP"):    return SIGTRAP;
		case hash("SIGABRT"):    return SIGABRT;
		case hash("SIGIOT"):     return SIGIOT;
		case hash("SIGBUS"):     return SIGBUS;
		case hash("SIGFPE"):     return SIGFPE;
		case hash("SIGKILL"):    return SIGKILL;
		case hash("SIGUSR1"):    return SIGUSR1;
		case hash("SIGSEGV"):    return SIGSEGV;
		case hash("SIGUSR2"):    return SIGUSR2;
		case hash("SIGPIPE"):    return SIGPIPE;
		case hash("SIGALRM"):    return SIGALRM;
		case hash("SIGTERM"):    return SIGTERM;
		case hash("SIGSTKFLT"):  return SIGSTKFLT;
		case hash("SIGCHLD"):    return SIGCHLD;
		case hash("SIGCONT"):    return SIGCONT;
		case hash("SIGSTOP"):    return SIGSTOP;
		case hash("SIGTSTP"):    return SIGTSTP;
		case hash("SIGTTIN"):    return SIGTTIN;
		case hash("SIGTTOU"):    return SIGTTOU;
		case hash("SIGURG"):     return SIGURG;
		case hash("SIGXCPU"):    return SIGXCPU;
		case hash("SIGXFSZ"):    return SIGXFSZ;
		case hash("SIGVTALRM"):  return SIGVTALRM;
		case hash("SIGPROF"):    return SIGPROF;
		case hash("SIGWINCH"):   return SIGWINCH;
		case hash("SIGIO"):      return SIGIO;
		case hash("SIGPWR"):     return SIGPWR;
		case hash("SIGSYS"):     return SIGSYS;
	}
}


inline
const char *reflect_signal(const int &val)
{
	switch(val)
	{
		case SIGHUP:      return "SIGHUP";
		case SIGINT:      return "SIGINT";
		case SIGQUIT:     return "SIGQUIT";
		case SIGILL:      return "SIGILL";
		case SIGTRAP:     return "SIGTRAP";
		case SIGABRT:     return "SIGABRT";
//		case SIGIOT:      return "SIGIOT";
		case SIGBUS:      return "SIGBUS";
		case SIGFPE:      return "SIGFPE";
		case SIGKILL:     return "SIGKILL";
		case SIGUSR1:     return "SIGUSR1";
		case SIGSEGV:     return "SIGSEGV";
		case SIGUSR2:     return "SIGUSR2";
		case SIGPIPE:     return "SIGPIPE";
		case SIGALRM:     return "SIGALRM";
		case SIGTERM:     return "SIGTERM";
		case SIGSTKFLT:   return "SIGSTKFLT";
		case SIGCHLD:     return "SIGCHLD";
		case SIGCONT:     return "SIGCONT";
		case SIGSTOP:     return "SIGSTOP";
		case SIGTSTP:     return "SIGTSTP";
		case SIGTTIN:     return "SIGTTIN";
		case SIGTTOU:     return "SIGTTOU";
		case SIGURG:      return "SIGURG";
		case SIGXCPU:     return "SIGXCPU";
		case SIGXFSZ:     return "SIGXFSZ";
		case SIGVTALRM:   return "SIGVTALRM";
		case SIGPROF:     return "SIGPROF";
		case SIGWINCH:    return "SIGWINCH";
		case SIGIO:       return "SIGIO";
		case SIGPWR:      return "SIGPWR";
		case SIGSYS:      return "SIGSYS";
	}
}
