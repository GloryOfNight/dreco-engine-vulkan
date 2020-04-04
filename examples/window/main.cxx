#include "engine/engine.hxx"

#include <iostream>

int main()
{
	std::cout << "Hello there" << std::endl;
	engine e;
	e.runMainLoop();
	return 0;
}