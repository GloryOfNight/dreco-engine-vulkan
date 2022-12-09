#include "game.hxx"

#include "core/engine.hxx"

DRECO_API void registerGame(de::defaultObject<de::gf::game_instance>& defaultGameInstance)
{
	defaultGameInstance.init<launcher_gi>();
}