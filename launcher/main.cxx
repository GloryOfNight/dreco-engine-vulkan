#include "engine/engine.hxx"

#include "shader_compiler.hxx"

#include <cstdlib>
#include <filesystem>
#include <string>

// someday it might be a game
// or a editor
// for now its just engine launcher

int main()
{
	if (std::filesystem::exists(DRECO_SHADERS_SOURCE_DIR))
	{
		shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR);
	}

	engine Engine;
	const bool initRes = Engine.init();
	if (!initRes)
	{
		return 1;
	}

	Engine.run();
	return 0;
}