// program that help gather all shader files
// create list of .spv files and shader source code files that need to be recompiled
// output file that can be used as include() in CMakeLists.txt

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool findIsSourceShader(const std::string_view& stem)
{
	if (".vert" == stem)
		return true;
	else if (".frag" == stem)
		return true;
	else if (".geom" == stem)
		return true;

	return false;
}

int main(int argc, char* argv[])
{
	namespace fs = std::filesystem;

	if (argc < 3)
	{
		std::cerr << "expected args <shader_dir> <output_cmake_path>" << std::endl;
		return 1;
	}

	fs::path shaderDirPath = std::string(argv[1]);
	std::string outputCmakePath = argv[2];

	if (!fs::exists(shaderDirPath) || !fs::is_directory(shaderDirPath))
	{
		std::cerr << "provided <shader_dir> path not exists or not valid" << std::endl;
		return 1;
	}

	std::vector<std::string> totalShaderFiles;
	std::vector<fs::path> needToCompileShaderFiles;

	for (auto& p : fs::recursive_directory_iterator(shaderDirPath))
	{
		if (!p.is_regular_file())
		{
			continue;
		}

		const bool isSourceShaderFile = findIsSourceShader(p.path().extension().string());
		if (isSourceShaderFile)
		{
			const std::string binPathStr = p.path().generic_string() + ".spv";
			fs::path binPath = binPathStr;
			totalShaderFiles.push_back(binPathStr);

			if (fs::exists(binPath) && fs::is_regular_file(binPath))
			{
				const auto lastBinWriteTime = fs::last_write_time(binPath);
				const auto lastCodeWriteTime = fs::last_write_time(p);
				if (lastCodeWriteTime < lastBinWriteTime)
				{
					continue;
				}
			}
			needToCompileShaderFiles.push_back(p.path());
		}
	}

    std::ofstream cmakeFile;
    cmakeFile.open(outputCmakePath, std::ios::out | std::ios::trunc);

    cmakeFile << "set(SHADER_BINARY_FILES \n";
    for (auto& str : totalShaderFiles)
    {
        cmakeFile << '"' << str << '"' << '\n';
    }
    cmakeFile << ")\n\n";

	for (auto& path : needToCompileShaderFiles)
	{
		cmakeFile << "list(APPEND SHADER_COMPILE_PATH_LIST " << path.parent_path() << ")\n";
		cmakeFile << "list(APPEND SHADER_COMPILE_NAMES_LIST " << path.filename() << ")\n";
	}
    cmakeFile.close();

	return 0;
}