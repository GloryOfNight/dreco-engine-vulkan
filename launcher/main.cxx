#include "engine/engine.hxx"

// someday it might be a game
// or a editor
// for now its just engine launcher

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