#include "engine/engine.hxx"

int main()
{
	engine Engine;
	const bool initRes = Engine.init();
	if (!initRes)
	{
		return 1;
	}

	Engine.run();
	return 0;
}