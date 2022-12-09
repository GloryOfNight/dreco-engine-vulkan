#include "core/engine.hxx"

#include "dreco.hxx"
#include "shader_compiler.hxx"

extern "C++" void registerGame(de::engine*);

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
		return 1;
	}

	registerGame(&engine);

	if (const auto res = shader_compiler::attemptCompileShaders(DRECO_SHADERS_SOURCE_DIR, DRECO_SHADERS_BINARY_DIR); res != 0)
	{
		DE_LOG(Error, "Shader compilation failed with: %i", res);
	}

	engine.run();

	return 0;
}