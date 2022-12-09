#include "game.hxx"

#include "core/engine.hxx"

static de::engine* gEngine = nullptr;

DRECO_API void registerGame(de::engine* engine)
{
	gEngine = engine;
	gEngine->setGameInstance(de::gf::game_instance::unique(new launcher_gi()));
}