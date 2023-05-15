#pragma once
#include "log/log.hxx"
#include "platforms/platform.h"

#include <string>

#define DRECO_IMPLEMENT_PRIMARY_GAME_INSTANCE_CLASS(GameInstanceClass)          \
	extern "C++" DRECO_API de::gf::game_instance::unique __createGameInstance() \
	{                                                                           \
		return de::gf::game_instance::unique(new GameInstanceClass());          \
	}

// clang-format off
#define DRECO_API
#if PLATFORM_WINDOWS
	#if DRECO_LIBRARY_TYPE_SHARED
		#undef DRECO_API
		#define DRECO_API __declspec(dllexport)
	#endif
#endif
// clang-format on

#define DRECO_ASSET(name) (std::string(DRECO_ASSETS_DIR) + "/" + name)
#define DRECO_SHADER(name) (std::string(DRECO_SHADERS_BINARY_DIR) + "/" + name)

#define DRECO_IMPLEMENT_PRIMARY_GAME_INSTANCE_CLASS(GameInstanceClass)          \
	extern "C++" DRECO_API de::gf::game_instance::unique __createGameInstance() \
	{                                                                           \
		return de::gf::game_instance::unique(new GameInstanceClass());          \
	}