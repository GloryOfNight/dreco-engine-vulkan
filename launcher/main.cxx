#include "core/engine.hxx"

#include "dreco.hxx"
#include "shader_compiler.hxx"

extern "C++" void registerGame(de::defaultObject<de::gf::game_instance>&);

int main()
{
	de::engine Engine;
	const auto initRes = Engine.initialize();

	registerGame(Engine._defaultGameInstance);

	if (initRes == de::engine::init_res::ok)
	{
		if (const auto res = shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR); res != 0)
		{
			DE_LOG(Error, "Shader compilation failed with: %i", res);
		}

		const auto runRes = Engine.run();
		return runRes != de::engine::run_res::ok ? 1 : 0;
	}
	return 1;
}