#pragma once

#include <iostream>
#include <fstream>
#include <cstring> // for making std::memcmp available

#define SETTINGS_FILE "settings.bin"
#define AUTO_SAVE_SETTINGS true
#define AUTO_LOAD_SETTINGS true

struct Settings {
    bool useLabelColors = true;
    bool showDebug = false;
    bool followData = false;
	bool enableDebug = false;
    int activeGradientComputerIndex = 0;
	bool usePCA = false;
    // Feel free to add more settings here.
    // IMPORTANT: you can only add simple types: e.g. int, float, double, bool, etc.
    // No pointers, std::string, std::vector, etc.

};

void writeSettingsToFile(const Settings& settings);

Settings readSettingsFromFile();

bool compareSettings(const Settings& a, const Settings& b);
