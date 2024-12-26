#include "shader_compiler_tool/shader_compiler.hxx"

#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WINDOWS
#define PLATFORM_WINDOWS 1
#else 
#define PLATFORM_WINDOWS 0
#endif

static bool isStemShaderSource(const std::string_view& stem)
{
	if (".vert" == stem)
		return true;
	else if (".frag" == stem)
		return true;
	else if (".geom" == stem)
		return true;

	return false;
}

int shader_compiler::attemptCompileShaders(const std::string_view& srcShaderDir, const std::string_view& binShaderDir)
{
	std::cout << "- - - SHADER COMPILER TOOL - BEGIN" << std::endl;

	const auto beginTime = std::chrono::steady_clock::now();

	namespace fs = std::filesystem;
	const fs::path& shaderSrcDirPath = srcShaderDir;
	const fs::path& shaderBinDirPath = binShaderDir;

	std::cout << "shader_src_dir: " << shaderSrcDirPath.generic_string() << std::endl;
	if (!fs::exists(shaderSrcDirPath) || !fs::is_directory(shaderSrcDirPath))
	{
		std::cerr << "provided arg for shaders_src_dir path not exists or not valid" << std::endl;
		return 1;
	}

	std::cout << "shader_bin_dir: " << shaderBinDirPath.generic_string() << std::endl;
	if ((!fs::exists(shaderBinDirPath) || !fs::is_directory(shaderBinDirPath)))
	{
		try
		{
			if (!fs::create_directory(shaderBinDirPath))
			{
				std::cerr << "provided arg for shaders_bin_dir path not exists and failed to create directory" << std::endl;
				return 1;
			}
		}
		catch (fs::filesystem_error& error)
		{
			std::cerr << error.what() << std::endl;
			return 1;
		}
	}

	std::vector<std::string> totalShaderFiles;
	std::vector<fs::path> needToCompileShaderFiles;

	for (auto& p : fs::recursive_directory_iterator(shaderSrcDirPath))
	{
		if (!p.is_regular_file())
		{
			continue;
		}

		const bool isSourceShaderFile = isStemShaderSource(p.path().extension().string());
		if (isSourceShaderFile)
		{
			const std::string binPathStr = shaderBinDirPath.generic_string() + "/" + p.path().filename().generic_string() + ".spv";
			totalShaderFiles.push_back(binPathStr);

			const fs::path binPath = binPathStr;
			if (fs::exists(binPath) && fs::is_regular_file(binPath))
			{
				const auto lastBinWriteTime = fs::last_write_time(binPath);
				const auto lastCodeWriteTime = fs::last_write_time(p);
				if (lastCodeWriteTime < lastBinWriteTime)
				{
					std::cout << binPathStr << " OK" << std::endl;
					continue;
				}
				else
				{
					std::cout << binPathStr << " OLD" << std::endl;
				}
			}
			else
			{
				std::cout << binPathStr << " MISSING" << std::endl;
			}    
			needToCompileShaderFiles.push_back(p.path());
		}
	}

	{
		const std::string exeStem = PLATFORM_WINDOWS ? ".exe" : "";
		for (auto& path : needToCompileShaderFiles)
		{
			const std::string sourceShaderPath = path.generic_string();
			const std::string outputShaderPath = shaderBinDirPath.generic_string() + '/' + path.filename().generic_string() + ".spv";
			const std::string command = "glslc" + exeStem + ' ' + sourceShaderPath + " -o " + outputShaderPath + " --target-env=vulkan1.3";
			const int result = std::system(command.data());
			if (result == 0)
			{
				std::cout << outputShaderPath << " - COMPILED OK" << std::endl;
			}
			else 
			{
				return 1;
			}
		}
	}
	const auto endTime = std::chrono::steady_clock::now();

	std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() / 1000.0 << "ms" << std::endl;

	std::cout << " - - - SHADER COMPILER TOOL - END" << std::endl;
	return 0;
}