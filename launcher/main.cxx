#include "core/engine.hxx"

#include "dreco.hxx"
#include "shader_compiler.hxx"

int main()
{
	if (const auto res = shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR); res != 0)
	{
		DE_LOG(Error, "Shader compilation failed with: %i", res);
	}

	engine Engine;
	const int32_t initRes = Engine.initialize();
	if (initRes == 0)
	{
		const int32_t runStatus = Engine.run();
		return runStatus;
	}
	return initRes;
}