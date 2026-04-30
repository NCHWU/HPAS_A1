#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

namespace SavedSettings
{
	void setPreviouslyLoadedFile(std::string path);

	std::string getPreviouslyLoadedFile();
}