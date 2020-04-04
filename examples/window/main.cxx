#include <iostream>
#include "engine/engine.hxx"

int main() 
{
    std::cout << "Hello there" << std::endl;
    engine e;
    e.runMainLoop();
    return 0;
}