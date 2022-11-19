#pragma once
#include "core/utils/log.hxx"
#include "platforms/platform.h"

#include <string>

// clang-format off
#if PLATFORM_WINDOWS
	#if DRECO_LIBRARY_TYPE_SHARED
		#define DRECO_API __declspec(dllexport)
	#else 
		#define DRECO_API
	#endif
#endif
// clang-format on

#define DRECO_ASSET(name) (std::string(DRECO_ASSETS_DIR) + "/" + name)
#define DRECO_SHADER(name) (std::string(DRECO_SHADERS_BINARY_DIR) + "/" + name)