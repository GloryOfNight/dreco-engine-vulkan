#include "core/engine.hxx"

#include "dreco.hxx"
#include "shader_compiler.hxx"

extern "C++" void registerGame(defaultObject<game_instance>&);

int main()
{
	engine Engine;
	const int32_t initRes = Engine.initialize();

	registerGame(Engine._defaultGameInstance);

	if (initRes == 0)
	{
		if (const auto res = shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR); res != 0)
		{
			DE_LOG(Error, "Shader compilation failed with: %i", res);
		}

		const int32_t runStatus = Engine.run();
		return runStatus;
	}
	return initRes;
}