#include "Settings.h"

void writeSettingsToFile(const Settings& settings) {
    std::ofstream outFile(SETTINGS_FILE, std::ios::binary);
    if (outFile) {
        outFile.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
        outFile.close();
    }
    else {
        std::cerr << "Error opening settings file for writing." << std::endl;
    }
}

Settings readSettingsFromFile() {
    Settings settings{};
    std::ifstream inFile(SETTINGS_FILE, std::ios::binary);
    if (inFile) {
        inFile.read(reinterpret_cast<char*>(&settings), sizeof(Settings));
        inFile.close();
    }
    else {
        std::cerr << "Error opening settings file for reading." << std::endl;
    }
    return settings;
}

bool compareSettings(const Settings& a, const Settings& b) {
    // Compare the settings structure byte by byte
    return std::memcmp(&a, &b, sizeof(Settings)) == 0;
}

