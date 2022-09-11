#pragma once

#ifdef __linux__
#define PLATFORM_LINUX true
#else 
#define PLATFORM_LINUX false
#endif

#ifdef __WIN32__
#define PLATFORM_WINDOWS true
#else
#define PLATFORM_WINDOWS false
#endif

#include "platforms/paths/generic.hxx"
using platform_paths = generic_paths;