#include "game.hxx"

#include "core/engine.hxx"

DRECO_API void registerGame(defaultObject<game_instance>& defaultGameInstance)
{
	defaultGameInstance.init<launcher_gi>();
}