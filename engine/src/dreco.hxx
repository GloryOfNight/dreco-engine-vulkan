#pragma once
#include "platforms/platform.h"
#include "core/utils/log.hxx"

#include <string>

#ifndef DRECO_API
#if _WIN32
#if DRECO_BUILD_SHARED
#define DRECO_API __declspec(dllexport)
#else
#define DRECO_API
#endif
#else
#define DRECO_API
#endif
#endif

#define DRECO_ASSET(name) (std::string(DRECO_ASSETS_DIR) + "/" + name)
#define DRECO_SHADER(name) (std::string(DRECO_SHADERS_BINARY_DIR) + "/" + name)