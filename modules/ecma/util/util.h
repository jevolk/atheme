/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */

#include "colors.h"                              // mIRC color enumeration
#include "tool.h"                                // General non-v8 utilities
#include "allocator.h"                           // Provides that dirty mowgli memory
#include "isolate.h"                             // Stacks a tls pointer for the ubiquitious Isolate
#include "context_scope.h"                       // Stacks a tls pointer for v8::Context
#include "handle_scope.h"                        // Convenience wrapper for v8::HandleScope
#include "trycatch_scope.h"                      // Convenience wrapper for v8::TryCatch
#include "reflect.h"                             // Some v8 reflections
#include "test.h"                                // Basic macros for v8
#include "ident.h"                               // Basic macros for v8
#include "ctx.h"                                 // Basic macros for v8
#include "privdata.h"                            // Basic macros for v8
#include "return_value.h"                        // Basic macros for v8
#include "string.h"                              // Basic macros for v8
#include "misc.h"                                // Basic macros for v8
#include "debug.h"                               // Some debugging tools
#include "exception.h"                           // Exception hierarchy
#include "maybe.h"                               // Utilities to bring v8 down to the mere mortal masses
#include "as.h"                                  // Utilities to bring v8 down to the mere mortal masses
#include "priv.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "own.h"                                 // Utilities to bring v8 down to the mere mortal masses
#include "real.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "data.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "internal.h"                            // Utilities to bring v8 down to the mere mortal masses
#include "path.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "has.h"                                 // Utilities to bring v8 down to the mere mortal masses
#include "get.h"                                 // Utilities to bring v8 down to the mere mortal masses
#include "proffer.h"                             // Utilities to bring v8 down to the mere mortal masses
#include "is_object.h"                           // Utilities to bring v8 down to the mere mortal masses
#include "set.h"                                 // Utilities to bring v8 down to the mere mortal masses
#include "call.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "dtor.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "ctor.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "del.h"                                 // Utilities to bring v8 down to the mere mortal masses
#include "keys.h"                                // Utilities to bring v8 down to the mere mortal masses
#include "for_each.h"                            // Utilities to bring v8 down to the mere mortal masses
#include "remove_if.h"                           // Utilities to bring v8 down to the mere mortal masses
#include "v8extended.h"                          // Utilities to bring v8 down to the mere mortal masses
#include "json.h"                                // json utils
