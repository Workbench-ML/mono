#include <mono/metadata/unity-baselib.h>
#include <mono/metadata/unity-utils.h>
#include <mono/utils/mono-dl.h>
#include <mono/utils/mono-logger-internals.h>

PAL_Identification_GetPlatformName_Type PAL_Identification_GetPlatformName = NULL;

static MonoDl *baselib_handle = NULL;

#define LOAD_BASELIB_FUNCTION(name) \
	char* baselib_function_load_error = load_baselib_function (#name, (gpointer) &name); \
	g_free (baselib_function_load_error);

#define UNLOAD_BASELIB_FUNCTION(name) name = NULL;

#define BASELIB_LIBRARY_NAME "baselib"

static char*
get_default_baselib_path ()
{
#if defined(HOST_WIN32)
	const char* baselib_filename = BASELIB_LIBRARY_NAME".dll";
#elif defined (TARGET_OSX)
	const char* baselib_filename = BASELIB_LIBRARY_NAME".dylib";
#else
	const char* baselib_filename = BASELIB_LIBRARY_NAME".so";
#endif

	const char *root_directory = mono_unity_get_baselib_directory ();
	size_t root_directory_length = strlen (root_directory);
	size_t baselib_path_length = root_directory_length + 1 + strlen(baselib_filename) + 1;
	char* baselib_path = (char *)g_malloc (baselib_path_length);
	snprintf(baselib_path, baselib_path_length, "%s/%s", root_directory, baselib_filename);

	return baselib_path;
}

static char*
load_baselib_function (const char *name, void **symbol)
{
	if (baselib_handle == NULL)
	{
		char *library_error_message = NULL;

		char* default_baselib_path = get_default_baselib_path ();
		baselib_handle = mono_dl_open (default_baselib_path, MONO_DL_LAZY, &library_error_message);
		if (baselib_handle == NULL)
		{
			UnityFindPluginCallback find_plugin = mono_get_find_plugin_callback ();
			if (find_plugin != NULL)
			{
				const char* unity_baselib_path = find_plugin (BASELIB_LIBRARY_NAME);
				baselib_handle = mono_dl_open (unity_baselib_path, MONO_DL_LAZY, &library_error_message);
				if (baselib_handle == NULL)
					mono_trace_message (MONO_TRACE_TYPE, "Unable to load the baselib dynamic library at '%s', error: '%s'", unity_baselib_path, library_error_message);
			}
		}

		if (baselib_handle == NULL)
		{
			mono_trace_message (MONO_TRACE_TYPE, "Unable to load the baselib dynamic library at '%s', error: '%s'", default_baselib_path, library_error_message);
			g_free(default_baselib_path);
			return library_error_message;
		}

		g_free(default_baselib_path);
	}

	char *function_error_message = NULL;
	function_error_message = mono_dl_symbol (baselib_handle, name, symbol);

	if (function_error_message != NULL)
		mono_trace_message (MONO_TRACE_TYPE, "Unable to load the function %s from baselib, error: '%s'", name, function_error_message);

	return function_error_message;
}

void
unity_baselib_init ()
{
	static gboolean inited = FALSE;

	if (inited)
		return;
	inited = TRUE;

	LOAD_BASELIB_FUNCTION (PAL_Identification_GetPlatformName)
}

void
unity_baselib_cleanup ()
{
	UNLOAD_BASELIB_FUNCTION (PAL_Identification_GetPlatformName)

	if (baselib_handle != NULL)
		mono_dl_close (baselib_handle);
}