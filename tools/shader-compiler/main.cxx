// program that help gather all shader files that need to be compiled and compile them
#include "include/shader_compiler.hxx"

#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "expected 2 args: shaders_src_dir, shaders_bin_dir" << std::endl;
		return 1;
	}
	return shader_compiler::attemptCompileShaders(argv[1], argv[2]);
}