#include <mono/metadata/profiler.h>
#include <mono/metadata/mono-debug.h>

#include "debugger-agent.h"

typedef void *MonoLegacyProfiler;
typedef void (*MonoLegacyProfileFunc) (MonoLegacyProfiler *prof);

MONO_API void mono_profiler_install (MonoLegacyProfiler *prof, MonoLegacyProfileFunc callback);

#define DEFINED_LEGACY_PROFILER

#include "../../dnSpyFiles/dnSpy.c"