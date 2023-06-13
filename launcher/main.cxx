#include "core/engine.hxx"
#include "game_framework/game_instance.hxx"

#include "dreco.hxx"
#include "core/misc/exceptions.hxx"
#include "shader_compiler_tool/shader_compiler.hxx"

extern "C++" DRECO_API de::gf::game_instance::unique __createGameInstance();

int main()
{
	de::engine engine;
	try
	{
		engine.initialize();
	}
	catch (de::except::initialization_error& e)
	{
		DE_LOG(Error, "%s: %s", __FUNCTION__, e.what());
		return EXIT_FAILURE;
	}

	engine.setCreateGameInstanceFunc(__createGameInstance);

	if (const auto res = shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR); res != 0)
	{
		DE_LOG(Error, "Shader compilation failed with: %i", res);
	}

	engine.run();

	return EXIT_SUCCESS;
}