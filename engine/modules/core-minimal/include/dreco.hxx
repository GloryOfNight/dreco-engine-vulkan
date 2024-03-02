#pragma once
#include "log/log.hxx"
#include "platforms/platform.h"

#include <string>

#define DRECO_IMPLEMENT_PRIMARY_GAME_INSTANCE_CLASS(GameInstanceClass)          \
	extern "C++" DRECO_API de::gf::game_instance::unique __createGameInstance() \
	{                                                                           \
		return de::gf::game_instance::unique(new GameInstanceClass());          \
	}

#if PLATFORM_WINDOWS
#define DRECO_API __declspec(dllexport)
#else 
#define DRECO_API 
#endif

#define DRECO_ASSET(name) (std::string(DRECO_ASSETS_DIR) + "/" + name)
#define DRECO_SHADER(name) (std::string(DRECO_SHADERS_BINARY_DIR) + "/" + name)